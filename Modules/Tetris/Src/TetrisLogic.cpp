#include "TetrisLogic.hpp"
using namespace tetris;
TetrisManager::TetrisManager()
{
	field.resize(ROW_OF_FIELD * COL_OF_FIELD);
	myFigure = CreateFigures(*this);
	nextFigure = myFigure.begin() + (HAL_GetTick() % QUANTITY_FIGURE);
	GenerateNewFigure();
}

size_t TetrisManager::EraseFilledRow()
{
	auto itBeg = field.begin();
	auto itEnd = itBeg;
	size_t ErasedRows = 0;
	size_t row = 0;
	for (; itBeg != field.end(); itBeg += COL_OF_FIELD, ++row)
	{
		itEnd += COL_OF_FIELD;
		if (std::all_of(itBeg, itEnd, [](auto& block)
			{return block.isFeeled; }))
		{
			field.erase(itBeg, itEnd);
			field.insert(field.begin(), COL_OF_FIELD, {});
			itBeg = field.begin() + (COL_OF_FIELD * row);
			itEnd = itBeg + COL_OF_FIELD;
			++ErasedRows;
		}
	}
	return ErasedRows;
}

void TetrisManager::GenerateNewFigure()
{
	static std::size_t counter = 1;
	currentFigure = nextFigure;
	nextFigure = myFigure.begin() + ((HAL_GetTick() + ++counter) % QUANTITY_FIGURE);
}
TetrisFigure::TetrisFigure(block_t&& figure_, Coordinate coordFigure_, size_t sideFigure_, color_t colorFigure, TetrisManager& manager_)
	: figure(std::move(figure_)), initFigure(figure), initCoord(coordFigure_),  prevCoord(coordFigure_),
	  coord(coordFigure_), manager(manager_), side(sideFigure_)
{
	for(int i = 0; i < pow(side, 2); ++i)
		if (figure[i].isFeeled)
		{
			figure[i].color = colorFigure;
			initFigure[i].color = colorFigure;
		}
}

void TetrisFigure::Rotate()
{
	prevFigure = figure;
	prevCoord = coord;
	auto counter = 1;
	size_t row = side - counter;
	std::for_each(figure.begin(), figure.end(), [&](auto& block)
		{block = prevFigure[row]; row += side; if (row > pow(side, 2)) row = side - ++counter; });
	while (CheckBoundaries());
	if (isIntersectionBlocks())
	{
		figure = prevFigure;
		coord = prevCoord;
	}
}
void TetrisFigure::MoveHorizontally(Directions direction)
{
	prevFigure = figure;
	prevCoord = coord;
	switch (direction)
	{
	case Directions::LEFT: --coord.X; break;
	case Directions::RIGHT: ++coord.X; break;
	default: return;
	}
	CheckBoundaries();
	if (isIntersectionBlocks()) coord = prevCoord;
}
bool TetrisFigure::MoveDown()
{
	prevFigure = figure;
	prevCoord = coord;
	++coord.Y;
	bool returnCB = CheckBoundaries();
	bool returnIB = isIntersectionBlocks();
	if (returnIB) --coord.Y;
	if (returnIB || returnCB)
	{
		if (CheckEndGame()) return true;
		FillFieldFigure();
		figure = initFigure;
		coord = initCoord;
		return false;
	}
	else return true;
}

bool TetrisFigure::CheckBoundaries()
{
	bool returnStatus = false;
	if ((coord.Y + side - 1) >= ROW_OF_FIELD)
	{
		if (isContainsUnitInRow((ROW_OF_FIELD - (coord.Y + (side - 1)) + (side - 1))))
		{
			coord.Y -= 1; returnStatus = true;
		}
	}
	if (coord.X < 0)
	{
		if (isContainsUnitInColumne(abs(coord.X) - 1))
		{
			coord.X += 1; returnStatus = true;
		}
	}
	else if ((coord.X + side - 1) >= COL_OF_FIELD)
	{
		if (isContainsUnitInColumne((COL_OF_FIELD - (coord.X + (side - 1)) + (side - 1))))
		{
			coord.X -= 1; returnStatus = true;
		}
	}
	return returnStatus;
}

bool TetrisFigure::isContainsUnitInColumne(size_t icol)
{
	size_t irow = 0;
	auto lambd1 = [&](auto& block)
	{
		if (irow == side) irow = 0;
		if (irow++ == icol) return !block.isFeeled;
		else return true;
	};
	return !std::all_of(figure.begin(), figure.end(), lambd1);
}
bool TetrisFigure::isContainsUnitInRow(size_t irow)
{
	size_t counter = 0;
	auto lambd2 = [&](auto& block)
	{
		if (irow == size_t(counter++ / side)) return !block.isFeeled;
		else return true;
	};
	return !std::all_of(figure.begin(), figure.end(), lambd2);
}
bool TetrisFigure::isIntersectionBlocks()
{
	size_t row = 0, col = 0;
	bool flag = false;
	auto lambd3 = [&](auto& block)
	{
		flag = false;
		if (block.isFeeled) {
			if (manager.field[((coord.Y + row) * COL_OF_FIELD) + coord.X + col].isFeeled)
				flag = true;
		}
		if (++col == side) { ++row, col = 0; }
		return flag;
	};
	return std::any_of(figure.begin(), figure.end(), lambd3);
}

void TetrisFigure::FillFieldFigure()
{
	size_t row = 0, col = 0;
	auto lambd4 = [&](auto& block)
	{
		if (block.isFeeled)
		{
			auto index = ((coord.Y + row) * COL_OF_FIELD) + coord.X + col;
			manager.field[index].isFeeled = true;
			manager.field[index].color = block.color;
		}
		if (++col == side) { ++row, col = 0; }
	};
	std::for_each(figure.begin(), figure.end(), lambd4);
}

bool TetrisFigure::CheckEndGame()
{
	if (coord.Y < HIDDEN_ROW_OF_FIELD)
	{
		if (isContainsUnitInRow((HIDDEN_ROW_OF_FIELD - 1) - coord.Y))
		{
			manager.END_OF_GAME = true;
			return true;
		}
	}
	return false;
}
std::vector<TetrisFigure> CreateFigures(TetrisManager& man)
{
	block_t fig1
	{
		{0},{0},{0},{0},
		{0},{0},{0},{0},
		{1},{1},{1},{1},
		{0},{0},{0},{0}
	};
	block_t fig2
	{
		{0},{0},{0},
		{1},{1},{1},
		{1},{0},{0}
	};
	block_t fig3
	{
		{0},{0},{0},
		{1},{1},{1},
		{0},{0},{1}
	};
	block_t fig4
	{
		{0},{0},{0},
		{0},{1},{1},
		{1},{1},{0}
	};
	block_t fig5
	{
		{0},{0},{0},
		{1},{1},{0},
		{0},{1},{1}
	};
	block_t fig6
	{
		{0},{0},{0},
		{1},{1},{1},
		{0},{1},{0}
	};
	block_t fig7
	{
		{1},{1},
		{1},{1}
	};
	std::vector<TetrisFigure> myFigure;
	myFigure.reserve(QUANTITY_FIGURE);
	myFigure.emplace_back(std::move(fig1), Coordinate{ 3, 2 }, 4, FIG1COLOR, man);
	myFigure.emplace_back(std::move(fig2), Coordinate{ 3, 2 }, 3, FIG2COLOR, man);
	myFigure.emplace_back(std::move(fig3), Coordinate{ 3, 2 }, 3, FIG3COLOR, man);
	myFigure.emplace_back(std::move(fig4), Coordinate{ 3, 2 }, 3, FIG4COLOR, man);
	myFigure.emplace_back(std::move(fig5), Coordinate{ 3, 2 }, 3, FIG5COLOR, man);
	myFigure.emplace_back(std::move(fig6), Coordinate{ 3, 2 }, 3, FIG6COLOR, man);
	myFigure.emplace_back(std::move(fig7), Coordinate{ 3, 3 }, 2, FIG7COLOR, man);
	return myFigure;
}
