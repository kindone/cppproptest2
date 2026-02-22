# Advanced Stateful & Concurrency Testing


!!! STUB "This page is work in progress"

Stateful testing opens new possibilities for testing.

* Finding memory leaks
    * With help of a suitable allocation tracker, we can place postcondition check that allocation numbers should come back to 0, after various actions executed and then destructed the object.
* Testing with various responses from external dependencies
    * Complex behavior of mocks can be generated with random sequence of the function specification constructs like ON_CALL of Google Test.
* Malfunctions (I/O failures, Network errors, Out-of-memory errors)
    * With help of a probabilistic malfunction generation functionality we can test for malfunction tolerance of a component, whether it leads to inconsistent states or even crashes
    * A typical malfunction generation functionality may be bound to a resource allocator (OOM) or I/O PIs (network, disk, etc.) that highly depends on external factors
    * A malfunction object may take a floating point argument that decides the probability of malfunctions can happen within the scope
        * Inside an allocator or I/O API the number set in the malfunction object is read and throws exception if the outcome of the dice is true.
* Thread-safety (concurrency testing)
    * After various actions are executed in parallel, consistency of the object is validated as postcondition
* Testing for Transactional requirements in databases
    * Transactional visibility as invariant property
        * Multiple transactions can be maintained and when the same resource (e.g. table) is accessed concurrently, uncommitted changes made should be only visible to current transaction and not in the others.
    * Atomicity of a transaction
        * If a sequence of changes is executed, they should be either fully committed or fully rolled back
