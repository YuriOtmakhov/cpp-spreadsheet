#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <variant>

using namespace std::literals;

Sheet::~Sheet() = default;//{}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid())
        throw InvalidPositionException("Invalid position in SetCell"s);

    auto new_cell = std::make_unique<Cell>(*this, pos);
    new_cell->Set(std::move(text));

    if (!table_.count(pos.ToString())) {


    if (pos.col >= size_.cols){
        size_.cols = pos.col+1;
        cells_in_col_.resize(size_.cols,0);
    }

    if (pos.row >= size_.rows) {
        size_.rows = pos.row+1;
        cells_in_row_.resize(size_.rows,0);
    }

    ++cells_in_col_[pos.col];
    ++cells_in_row_[pos.row];
    }else {
        for (auto item : dynamic_cast<Cell*>(table_.at(pos.ToString()).get())->GetParentCells())
            new_cell->AddPerents(item);
    }

    table_[pos.ToString()] = std::move(new_cell);

}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid())
        throw InvalidPositionException("Invalid position in GetCell"s);

    if (table_.find(pos.ToString()) != table_.end())
        return table_.at(pos.ToString()).get();

    return nullptr;

}
CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid())
        throw InvalidPositionException("Invalid position in GetCell"s);

    if (table_.find(pos.ToString()) != table_.end())
        return table_.at(pos.ToString()).get();

    return nullptr;
}

void Sheet::RelaxedSize(){
    for (;cells_in_col_[size_.cols-1] == 0;--size_.cols)
        if (size_.cols == 1) {
            --size_.cols;
            break;
        }

    for (;cells_in_row_[size_.rows-1] == 0;--size_.rows)
        if (size_.rows == 1) {
            --size_.rows;
            break;
        }
    cells_in_col_.resize(size_.cols);
    cells_in_row_.resize(size_.rows);
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid())
        throw InvalidPositionException("Invalid position in ClearCell"s);

    if (table_.find(pos.ToString()) == table_.end())
        return;

    table_.erase(pos.ToString());
    --cells_in_col_[pos.col];
    --cells_in_row_[pos.row];
    RelaxedSize();

}

Size Sheet::GetPrintableSize() const {
    return size_;
}

struct ValuePrinter {
    std::ostream& out;

    void operator()(double value) const {
        out << value;
    }
    void operator()(std::string value) const {
        out << value;
    }
    void operator()(FormulaError value) const {
        out << value;
    }

};

void Sheet::PrintValues(std::ostream& output) const {
    for (int i = 0; i < size_.rows; ++i){
        for (int j =0; j < size_.cols; ++j) {
            if (j)
                output<<'\t';

            if (std::string pos_name = Position{i,j}.ToString(); table_.find(pos_name) != table_.end())
                std::visit(ValuePrinter{output}, table_.at(pos_name)->GetValue());
        }
        output<<'\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    for (int i = 0; i < size_.rows; ++i){
        for (int j =0; j < size_.cols; ++j) {
            if (j)
                output<<'\t';

            if (std::string pos_name = Position{i,j}.ToString(); table_.find(pos_name) != table_.end())
                output<< table_.at(pos_name)->GetText();
        }
        output<<'\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
