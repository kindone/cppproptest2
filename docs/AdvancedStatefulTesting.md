# Advanced Stateful & Concurrency Testing

> **New to stateful testing?** Start with [Stateful Testing](StatefulTesting.md) for the basics. This page covers advanced techniques for complex scenarios.

Stateful testing opens new possibilities for testing beyond simple state transitions. This guide covers advanced patterns for detecting resource leaks, handling external dependencies, simulating failures, and verifying transactional properties.

---

## Finding Memory Leaks

With help of a suitable allocation tracker, we can place postcondition check that allocation numbers should come back to 0, after various actions executed and then destructed the object.

### Using Allocation Tracker

An allocation tracker monitors memory allocations and deallocations. After executing a sequence of actions and destroying the object, you can verify that all allocations were properly freed:

```cpp
// AllocationTracker: getCurrentCount() -> size_t

class AllocationTracker {
public:
    static size_t getCurrentCount() { /* ... */ }
    // Tracks all allocations/deallocations
};

// ResourceManager: allocate(id), deallocate(id), ~ResourceManager()

auto allocateGen = gen::int32().map<SimpleAction<ResourceManager>>([](int id) {
    return SimpleAction<ResourceManager>([id](ResourceManager& mgr) {
        mgr.allocate(id);
    });
});

auto deallocateGen = gen::int32().map<SimpleAction<ResourceManager>>([](int id) {
    return SimpleAction<ResourceManager>([id](ResourceManager& mgr) {
        mgr.deallocate(id);
    });
});

auto actionGen = gen::oneOf<SimpleAction<ResourceManager>>(allocateGen, deallocateGen);

auto prop = statefulProperty<ResourceManager>(
    gen::lazy<ResourceManager>([]() { return ResourceManager(); }),
    actionGen
);

prop.setOnStartup([]() {
    // Reset allocation tracker before each run
    AllocationTracker::reset();
});

prop.setPostCheck([](ResourceManager& mgr) {
    // Destroy the object and check allocations
    mgr.~ResourceManager();
    size_t remainingAllocations = AllocationTracker::getCurrentCount();
    PROP_ASSERT_EQ(remainingAllocations, 0);  // Should be 0 after destruction
});

prop.setNumRuns(1000).go();
```

### Tracking Across Action Sequences

You can also track allocations throughout the action sequence:

```cpp
struct AllocationModel {
    size_t expectedAllocations = 0;
};

auto allocateGen = gen::int32().map<Action<ResourceManager, AllocationModel>>([](int id) {
    return Action<ResourceManager, AllocationModel>([id](ResourceManager& mgr, AllocationModel& model) {
        size_t before = AllocationTracker::getCurrentCount();
        mgr.allocate(id);
        size_t after = AllocationTracker::getCurrentCount();
        model.expectedAllocations += (after - before);
    });
});

auto deallocateGen = gen::int32().map<Action<ResourceManager, AllocationModel>>([](int id) {
    return Action<ResourceManager, AllocationModel>([id](ResourceManager& mgr, AllocationModel& model) {
        size_t before = AllocationTracker::getCurrentCount();
        mgr.deallocate(id);
        size_t after = AllocationTracker::getCurrentCount();
        model.expectedAllocations -= (before - after);
    });
});

prop.setPostCheck([](ResourceManager& mgr, AllocationModel& model) {
    mgr.~ResourceManager();
    size_t actual = AllocationTracker::getCurrentCount();
    PROP_ASSERT_EQ(actual, 0);
    PROP_ASSERT_EQ(model.expectedAllocations, 0);
});
```

**See also:** [Property API Reference - Configuration](Property.md#configuration) for `setOnStartup` and `setOnCleanup` details.

---

## Testing with Various Responses from External Dependencies

Complex behavior of mocks can be generated with random sequence of the function specification constructs like `ON_CALL` of Google Test.

### Generating Mock Behaviors

You can generate random sequences of `ON_CALL` specifications to test how your system handles various external dependency responses:

```cpp
// ExternalService: fetchData(id) -> Data, saveData(id, data) -> bool

class MockExternalService {
public:
    MOCK_METHOD(Data, fetchData, (int id));
    MOCK_METHOD(bool, saveData, (int id, const Data& data));
};

auto setupMockGen = gen::oneOf(
    // Success response
    gen::just(SimpleAction<MockExternalService>([](MockExternalService& mock) {
        ON_CALL(mock, fetchData(_)).WillByDefault(Return(Data{42}));
        ON_CALL(mock, saveData(_, _)).WillByDefault(Return(true));
    })),
    // Failure response
    gen::just(SimpleAction<MockExternalService>([](MockExternalService& mock) {
        ON_CALL(mock, fetchData(_)).WillByDefault(Throw(std::runtime_error("Network error")));
        ON_CALL(mock, saveData(_, _)).WillByDefault(Return(false));
    })),
    // Partial failure
    gen::just(SimpleAction<MockExternalService>([](MockExternalService& mock) {
        ON_CALL(mock, fetchData(_)).WillByDefault(Return(Data{42}));
        ON_CALL(mock, saveData(_, _)).WillByDefault(Return(false));
    })),
    // Delayed response
    gen::just(SimpleAction<MockExternalService>([](MockExternalService& mock) {
        ON_CALL(mock, fetchData(_)).WillByDefault(DoAll(
            SleepFor(std::chrono::milliseconds(100)),
            Return(Data{42})
        ));
    }))
);

auto actionGen = gen::oneOf<SimpleAction<MockExternalService>>(setupMockGen);

auto prop = statefulProperty<MockExternalService>(
    gen::lazy<MockExternalService>([]() { return MockExternalService(); }),
    actionGen
);

prop.setPostCheck([](MockExternalService& mock) {
    // Verify system handled all mock responses correctly
    // Check for consistent state regardless of mock behavior
});
```

### State-Dependent Mock Configuration

Generate mock behaviors based on current system state:

```cpp
auto mockConfigGen = [](const SystemState& state, const SystemModel& model) {
    vector<Generator<Action<SystemState, SystemModel>>> configs;
    
    if (model.connectionCount == 0) {
        // No connections - simulate connection failures
        configs.push_back(gen::just(Action<SystemState, SystemModel>("MockConnectionFailure",
            [](SystemState& state, SystemModel& model) {
                ON_CALL(*state.mockService, connect(_))
                    .WillByDefault(Throw(std::runtime_error("Connection refused")));
            }
        )));
    } else {
        // Has connections - simulate various responses
        configs.push_back(gen::oneOf(
            gen::just(Action<SystemState, SystemModel>("MockSuccess", ...)),
            gen::just(Action<SystemState, SystemModel>("MockTimeout", ...)),
            gen::just(Action<SystemState, SystemModel>("MockPartialData", ...))
        ));
    }
    
    return gen::oneOf<Action<SystemState, SystemModel>>(configs);
};
```

---

## Malfunctions (I/O failures, Network errors, Out-of-memory errors)

With help of a probabilistic malfunction generation functionality we can test for malfunction tolerance of a component, whether it leads to inconsistent states or even crashes.

A typical malfunction generation functionality may be bound to a resource allocator (OOM) or I/O APIs (network, disk, etc.) that highly depends on external factors.

A malfunction object may take a floating point argument that decides the probability of malfunctions can happen within the scope. Inside an allocator or I/O API the number set in the malfunction object is read and throws exception if the outcome of the dice is true.

### Probabilistic Malfunction Generator

Create a malfunction object that controls failure probability:

```cpp
class MalfunctionController {
public:
    MalfunctionController(double failureProbability) 
        : probability(failureProbability), rng(std::random_device{}()) {}
    
    bool shouldFail() {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        return dist(rng) < probability;
    }
    
    void setProbability(double prob) { probability = prob; }
    
private:
    double probability;
    std::mt19937 rng;
};

// Custom allocator with malfunction injection
template<typename T>
class MalfunctionAllocator {
public:
    MalfunctionAllocator(MalfunctionController& controller) : controller(controller) {}
    
    T* allocate(size_t n) {
        if (controller.shouldFail()) {
            throw std::bad_alloc();
        }
        return static_cast<T*>(::operator new(n * sizeof(T)));
    }
    
private:
    MalfunctionController& controller;
};
```

### Testing with Probabilistic Failures

Generate actions that set malfunction probability and test system resilience:

```cpp
// System: allocateResource(size), deallocateResource(id), setMalfunctionController(controller)

auto setFailureProbabilityGen = gen::float64(0.0, 1.0).map<Action<System, SystemModel>>(
    [](double prob) {
        return Action<System, SystemModel>([prob](System& sys, SystemModel& model) {
            auto controller = std::make_shared<MalfunctionController>(prob);
            sys.setMalfunctionController(controller);
            model.failureProbability = prob;
        });
    }
);

auto allocateGen = gen::size().map<Action<System, SystemModel>>([](size_t size) {
    return Action<System, SystemModel>([size](System& sys, SystemModel& model) {
        try {
            sys.allocateResource(size);
            model.allocated += size;
        } catch (const std::bad_alloc&) {
            // Out of memory - system should handle gracefully
            model.oomCount++;
        } catch (const std::exception&) {
            // Other failures
            model.failureCount++;
        }
    });
});

auto actionGen = gen::oneOf<Action<System, SystemModel>>(
    setFailureProbabilityGen, allocateGen, deallocateGen
);

auto prop = statefulProperty<System, SystemModel>(
    gen::lazy<System>([]() { return System(); }),
    [](System&) { return SystemModel(); },
    actionGen
);

prop.setPostCheck([](System& sys, SystemModel& model) {
    // System should remain in consistent state despite failures
    PROP_ASSERT(sys.isConsistent());
    // Verify no resource leaks even after failures
    PROP_ASSERT_EQ(sys.getLeakedResources(), 0);
});
```

### I/O Malfunction Injection

Inject failures into I/O operations:

```cpp
class IOMalfunctionController {
public:
    IOMalfunctionController(double readFailureProb, double writeFailureProb)
        : readProb(readFailureProb), writeProb(writeFailureProb) {}
    
    void checkRead() {
        if (shouldFail(readProb)) {
            throw std::ios_base::failure("Read failure");
        }
    }
    
    void checkWrite() {
        if (shouldFail(writeProb)) {
            throw std::ios_base::failure("Write failure");
        }
    }
    
private:
    double readProb, writeProb;
    std::mt19937 rng{std::random_device{}()};
    
    bool shouldFail(double prob) {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        return dist(rng) < prob;
    }
};

// FileManager: readFile(name), writeFile(name, data), setIOMalfunctionController(controller)

auto setIOMalfunctionGen = gen::tuple(gen::float64(0.0, 1.0), gen::float64(0.0, 1.0))
    .map<Action<FileManager, FileModel>>([](const tuple<double, double>& probs) {
        return Action<FileManager, FileModel>(
            [readProb = get<0>(probs), writeProb = get<1>(probs)]
            (FileManager& mgr, FileModel& model) {
                auto controller = std::make_shared<IOMalfunctionController>(readProb, writeProb);
                mgr.setIOMalfunctionController(controller);
                model.ioFailureProb = {readProb, writeProb};
            }
        );
    });
```

---

## Thread-Safety (Concurrency Testing)

After various actions are executed in parallel, consistency of the object is validated as postcondition.

### Parallel Action Execution

Test thread-safety by executing actions in parallel and verifying consistency:

```cpp
// ConcurrentSystem: addItem(id), removeItem(id), getCount() -> size_t

auto addItemGen = gen::int32().map<SimpleAction<ConcurrentSystem>>([](int id) {
    return SimpleAction<ConcurrentSystem>([id](ConcurrentSystem& sys) {
        sys.addItem(id);
    });
});

auto removeItemGen = gen::int32().map<SimpleAction<ConcurrentSystem>>([](int id) {
    return SimpleAction<ConcurrentSystem>([id](ConcurrentSystem& sys) {
        sys.removeItem(id);
    });
});

auto actionGen = gen::oneOf<SimpleAction<ConcurrentSystem>>(addItemGen, removeItemGen);

// Use concurrency testing API
auto concurrentProp = concurrency<ConcurrentSystem>(
    gen::lazy<ConcurrentSystem>([]() { return ConcurrentSystem(); }),
    actionListGenOf<ConcurrentSystem>(actionGen)
);

concurrentProp.setPostCheck([](ConcurrentSystem& sys) {
    // Verify consistency after parallel execution
    size_t count = sys.getCount();
    PROP_ASSERT_GE(count, 0);  // Count should be non-negative
    
    // Verify no data corruption
    PROP_ASSERT(sys.isConsistent());
    
    // Verify all operations completed successfully
    PROP_ASSERT_EQ(sys.getFailedOperations(), 0);
});

concurrentProp.go();
```

### Model-Based Concurrency Testing

Track expected state in model while actions execute concurrently:

```cpp
struct ConcurrentModel {
    std::set<int> items;
    std::atomic<size_t> operationCount{0};
};

auto addItemGen = gen::int32().map<Action<ConcurrentSystem, ConcurrentModel>>([](int id) {
    return Action<ConcurrentSystem, ConcurrentModel>([id](ConcurrentSystem& sys, ConcurrentModel& model) {
        sys.addItem(id);
        // Note: Model updates may need synchronization in real implementation
        model.items.insert(id);
        model.operationCount++;
    });
});

concurrentProp.setPostCheck([](ConcurrentSystem& sys, ConcurrentModel& model) {
    // After all threads join, verify final state matches model
    size_t actualCount = sys.getCount();
    size_t expectedCount = model.items.size();
    
    // Due to concurrent execution, counts may differ slightly
    // But system should remain consistent
    PROP_ASSERT(sys.isConsistent());
    PROP_ASSERT_LE(std::abs(static_cast<int>(actualCount) - static_cast<int>(expectedCount)), 
                   model.operationCount.load());
});
```

**See also:** [Concurrency Testing](ConcurrencyTesting.md) for detailed concurrency testing patterns.

---

## Testing for Transactional Requirements in Databases

### Transactional Visibility as Invariant Property

Multiple transactions can be maintained and when the same resource (e.g. table) is accessed concurrently, uncommitted changes made should be only visible to current transaction and not in the others.

```cpp
// Database: beginTransaction(txId), insert(txId, key, value), commit(txId), rollback(txId), read(key) -> value

struct TransactionModel {
    std::map<int, std::map<std::string, int>> uncommittedChanges;  // txId -> {key -> value}
    std::map<std::string, int> committedData;
    std::set<int> activeTransactions;
};

auto beginTxGen = gen::int32().map<Action<Database, TransactionModel>>([](int txId) {
    return Action<Database, TransactionModel>([txId](Database& db, TransactionModel& model) {
        db.beginTransaction(txId);
        model.activeTransactions.insert(txId);
        model.uncommittedChanges[txId] = {};
    });
});

auto insertGen = gen::tuple(gen::int32(), gen::string(), gen::int32())
    .map<Action<Database, TransactionModel>>([](const tuple<int, string, int>& args) {
        return Action<Database, TransactionModel>(
            [txId = get<0>(args), key = get<1>(args), value = get<2>(args)]
            (Database& db, TransactionModel& model) {
                db.insert(txId, key, value);
                model.uncommittedChanges[txId][key] = value;
            }
        );
    });

auto commitGen = gen::int32().map<Action<Database, TransactionModel>>([](int txId) {
    return Action<Database, TransactionModel>([txId](Database& db, TransactionModel& model) {
        if (model.activeTransactions.count(txId)) {
            db.commit(txId);
            // Move uncommitted changes to committed
            for (const auto& [key, value] : model.uncommittedChanges[txId]) {
                model.committedData[key] = value;
            }
            model.uncommittedChanges.erase(txId);
            model.activeTransactions.erase(txId);
        }
    });
});

auto prop = statefulProperty<Database, TransactionModel>(
    gen::lazy<Database>([]() { return Database(); }),
    [](Database&) { return TransactionModel(); },
    gen::oneOf<Action<Database, TransactionModel>>(beginTxGen, insertGen, commitGen, rollbackGen)
);

prop.setPostCheck([](Database& db, TransactionModel& model) {
    // Verify visibility: uncommitted changes not visible to other transactions
    for (const auto& [txId, changes] : model.uncommittedChanges) {
        for (const auto& [key, value] : changes) {
            // This key should only be visible within transaction txId
            // Other transactions should see committed value or nothing
            int committedValue = model.committedData.count(key) 
                ? model.committedData.at(key) 
                : -1;
            
            // Verify isolation: uncommitted changes not visible outside transaction
            for (int otherTxId : model.activeTransactions) {
                if (otherTxId != txId) {
                    // Other transaction should not see this uncommitted change
                    int otherView = db.readFromTransaction(otherTxId, key);
                    PROP_ASSERT_NE(otherView, value);  // Should see committed or default
                }
            }
        }
    }
    
    // All active transactions should be isolated
    PROP_ASSERT(db.verifyIsolation(model.activeTransactions));
});
```

### Atomicity of a Transaction

If a sequence of changes is executed, they should be either fully committed or fully rolled back.

```cpp
auto prop = statefulProperty<Database, TransactionModel>(...);

prop.setPostCheck([](Database& db, TransactionModel& model) {
    // Verify atomicity: all-or-nothing property
    for (const auto& [txId, changes] : model.uncommittedChanges) {
        // If transaction is still active, changes should not be visible
        if (model.activeTransactions.count(txId)) {
            // None of the uncommitted changes should be in committed data
            for (const auto& [key, value] : changes) {
                if (model.committedData.count(key)) {
                    PROP_ASSERT_NE(model.committedData.at(key), value);
                }
            }
        }
    }
    
    // Verify: either all changes committed or none
    for (int txId : model.activeTransactions) {
        if (model.uncommittedChanges.count(txId)) {
            // Transaction still active - verify no partial commits
            for (const auto& [key, value] : model.uncommittedChanges[txId]) {
                // Should not appear in committed data
                bool partiallyCommitted = model.committedData.count(key) && 
                                         model.committedData.at(key) == value;
                PROP_ASSERT_FALSE(partiallyCommitted);
            }
        }
    }
    
    // After commit, all changes should be visible
    // After rollback, none should be visible
    for (const auto& [key, value] : model.committedData) {
        int dbValue = db.read(key);
        PROP_ASSERT_EQ(dbValue, value);  // Committed data should match
    }
});
```

### Testing Concurrent Transactions

Test multiple transactions accessing the same resource concurrently:

```cpp
// Use concurrency testing for multiple transactions
auto concurrentProp = concurrency<Database>(
    gen::lazy<Database>([]() { return Database(); }),
    actionListGenOf<Database>(transactionActionGen)
);

concurrentProp.setPostCheck([](Database& db) {
    // Verify transactional properties hold under concurrency:
    // 1. Isolation: uncommitted changes not visible to other transactions
    // 2. Atomicity: all-or-nothing commit/rollback
    // 3. Consistency: database remains in valid state
    
    PROP_ASSERT(db.verifyIsolation());
    PROP_ASSERT(db.verifyAtomicity());
    PROP_ASSERT(db.isConsistent());
});
```

---

## Related Topics

- [Stateful Testing](StatefulTesting.md) - Basic stateful testing concepts
- [Concurrency Testing](ConcurrencyTesting.md) - Testing concurrent stateful systems
- [Property API Reference](Property.md) - Configuration and assertion APIs
- [Generators](Generators.md) - Creating input generators
- [Combinators](Combinators.md) - Combining generators
