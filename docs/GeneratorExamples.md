# Generating Real-World Inputs

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

auto yearMonthGen = tupleOf(interval(0, 9999), interval(1,12));

// combines (int,int) and int into (int,int,int)
auto dateTupleGen = yearMonthGen.tupleWith([](const std::tuple<int, int> yearMonth) {
    int year = std::get<0>(yearMonth);
    int month = std::get<1>(yearMonth);
    auto dayGen = interval(1, getMaximumDay(year, month));
    return dayGen;
});

```

## Generating a fixed point decimal with precision `p` and scale `s`
* A fixed point decimal requires a scale and a digit with precision

## Generating a chess move
* A board configuration is given
* One of the pieces on the board is moved, according to the chess rule

```cpp

using GenT = std::pair<ChessBoard, ChessMove>;

auto genT = just(std::make_pair(
    ChessBoard::initialBoard(), // Function to create a starting chessboard
    ChessMove::nullMove()       // Placeholder for the initial move
));

auto gen2GenT = [](const std::pair<ChessBoard, ChessMove>& prev) {
  // 1. Extract the ChessBoard from 'prev'.
  const ChessBoard& currentBoard = prev.first;

  // 2. Generate a list of LEGAL moves from the 'currentBoard'.
  auto legalMoves = generateLegalMoves(currentBoard);

  // 3. Create a generator that selects from 'legalMoves'.
  auto moveGen = elementOf(legalMoves);

  // 4.  For each chosen 'move', apply it to 'currentBoard'
  //     to get the new board state.
  return moveGen.map([currentBoard](const ChessMove& move) {
    ChessBoard newBoard = currentBoard.applyMove(move);
    return std::make_pair(newBoard, move); 
  });
};

using GenT2GenT = decltype(gen2GenT);

auto chessMoveSequenceGen = accumulate<GenT, GenT2GenT>(genT, gen2GenT, minSize, maxSize);
```