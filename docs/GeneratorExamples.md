# Generating Real-World Inputs

Obtaining a suitable generator for your property test is crucial.
This page provides realistic examples of generating complex inputs.

## Generating a valid `Date`

* A proper Date representation based on number of days in a month
* A day of a month should be in range between 1 to 31, while considering which month and year it is, to have correct maximum value among 28, 29, 30, and 31.

```cpp
bool isLeapYear(int year) {
  if (year % 4 != 0) {
    return false;
  } else if (year % 100 == 0) {
    return (year % 400 == 0); // Century year check
  } else {
    return true;
  }
}

int getMaximumDay(int year, int month) {
    // February Handling with Leap Year Information
    if (month == 2) {
        return isLeapYear(year) ? 29 : 28;
    }
    // 30-day Months
    if (month == 4 || month == 6 || month == 9 || month == 11) {
        return 30;
    }
    // All other months have 31 days
    return 31;
}

auto yearMonthGen = gen::tuple(gen::interval(0, 9999), gen::interval(1,12));

// combines (int,int) and int into (int,int,int)
auto dateTupleGen = yearMonthGen.tupleWith([](const std::tuple<int, int> yearMonth) {
    int year = std::get<0>(yearMonth);
    int month = std::get<1>(yearMonth);
    auto dayGen = gen::interval(1, getMaximumDay(year, month));
    return dayGen;
});

```

*## Generating a fixed point decimal with precision `p` and scale `s`

* A `Decimal(p, s)` is a fixed point decimal with a precision `p` and a scale `s`.
* It means there are `p` number of digits with `s` number of decimals, like `123.45` being a `Decimal(5,2)`.
* We can first generate `p - s` number of numeric characters that does not start with `'0'`.
* Then we can generate rest `s` number of numeric characters and concatenate the two strings with `.`.
* We can generate a sign character in front of the new string to complete the decimal*

```cpp

    auto precisionGen = gen::interval(1, maxPrecision);
    auto scaleGen = gen::interval(minScale, maxScale);
    auto tupleGen = gen::tuple(precisionGen, scaleGen);

    auto decimalGen = tupleGen.template flatMap<Decimal>([](const std::tuple<int, int>& tup) -> GenFunction<federation::Decimal> {
        int precision = std::get<0>(tup);
        int scale = std::get<1>(tup);
        // decide digits of precision
        auto signGen = gen::boolean(); // true: neg, false: pos
        auto firstGen = gen::interval<char>('1', '9');
        auto stringGen = gen::string(gen::interval('0', '9'));
        stringGen.setSize(precision-1);
        auto decTupleGen = gen::tuple(signGen, firstGen, stringGen);
        return decTupleGen.map<federation::Decimal>([scale](const ltt::tuple<bool, char, _STL::string>& decTup) {
            bool isNeg = std::get<0>(decTup);
            char first = std::get<1>(decTup);
            std::string rest = std::get<2>(decTup);
            std::stringstream digits;
            digits << first << rest;
            return Decimal(digits.str(), scale, isNeg);
        });
    });

```

## Generating a chess move

* A legitimate chess move follows complex rules based on previous state and action
    * A board configuration is given as previous state
    * Last move is given as previous action

```cpp

using GenT = std::pair<ChessBoard, ChessMove>;

auto genT = gen::just(std::make_pair(
    ChessBoard::initialBoard(), // Function to create a starting chessboard
    ChessMove::nullMove()       // Placeholder for the initial move
));

auto gen2GenT = [](const std::pair<ChessBoard, ChessMove>& prev) {
  // 1. Extract the ChessBoard from 'prev'.
  const ChessBoard& currentBoard = prev.first;

  // 2. Generate a list of LEGAL moves from the 'currentBoard'.
  auto legalMoves = generateLegalMoves(currentBoard);

  // 3. Create a generator that selects from 'legalMoves'.
  auto moveGen = gen::elementOf(legalMoves);

  // 4.  For each chosen 'move', apply it to 'currentBoard'
  //     to get the new board state.
  return moveGen.map([currentBoard](const ChessMove& move) {
    ChessBoard newBoard = currentBoard.applyMove(move);
    return std::make_pair(newBoard, move);
  });
};

using GenT2GenT = decltype(gen2GenT);

auto chessMoveSequenceGen = gen::accumulate<GenT, GenT2GenT>(genT, gen2GenT, minSize, maxSize);
```
