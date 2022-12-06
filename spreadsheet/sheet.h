#pragma once

#include "cell.h"
#include "common.h"

#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <string>

class Sheet : public SheetInterface {
	Size size_;
    std::unordered_map<std::string,std::unique_ptr<CellInterface>> table_;
    std::vector<int> cells_in_row_;
    std::vector<int> cells_in_col_;


    void RelaxedSize();

public:
    Sheet() : cells_in_row_(std::vector<int>(Position::MAX_ROWS,0)), cells_in_col_(std::vector<int> (Position::MAX_COLS,0)) {
    };

    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

	// Можете дополнить ваш класс нужными полями и методами
};
