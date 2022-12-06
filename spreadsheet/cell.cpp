#include "cell.h"

#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <stack>
#include <unordered_map>


using namespace std::literals;

class Cell::Impl {
protected:
    std::string text_;
public:
    Impl() = default;
    Impl(std::string_view str);
    virtual ~Impl() = default;
    virtual CellInterface::Value GetValue(bool) = 0;
    virtual std::string GetText() const;
};

class Cell::EmptyImpl : public Impl{
public:
    EmptyImpl();
    CellInterface::Value GetValue(bool) override;
};

class Cell::TextImpl : public Impl{
    std::string_view value_;
public:
    TextImpl(std::string_view str);
    CellInterface::Value GetValue(bool) override;
};

class Cell::FormulaImpl : public Impl{
    Sheet& sheet_;
    std::unique_ptr<FormulaInterface> formula_;
    FormulaInterface::Value value_;
public:
    FormulaImpl(std::string_view str, Sheet& sheet, const Position& pos, std::vector<Position>& child_cells_);
    CellInterface::Value GetValue(bool) override;
};

Cell::Cell(Sheet& sheet, Position pos) : pos_(pos), sheet_(sheet){
}

Cell::~Cell() = default;

void Cell::Set(std::string text) {
    if (text.empty())
        impl_ = std::make_unique<EmptyImpl>();
    else if (text.front() == '=' && text.size() > 1)
        impl_ = std::make_unique<FormulaImpl>(text, sheet_, pos_, child_cells_);
    else
        impl_ = std::make_unique<TextImpl>(text);

    is_calculation_ = true;
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    return impl_ -> GetValue(is_calculation_);
}
std::string Cell::GetText() const {
    return impl_ -> GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return child_cells_;
}

bool Cell::IsReferenced() const {
    return !parant_cells_.empty();
}

std::vector<Position> Cell::GetParentCells() const{
    std::vector<Position> ans;
    ans.reserve(parant_cells_.size());
    for (auto item : parant_cells_)
        ans.push_back(item);
    return ans;
}

void Cell::DeletePerents(const Position& cell) {
    parant_cells_.erase(cell);
}

void Cell::AddPerents(const Position& cell) {
    parant_cells_.insert(cell);
}

Cell::Impl::Impl(std::string_view str) : text_(str) {
}

std::string Cell::Impl::GetText() const {
    return text_;
}

Cell::EmptyImpl::EmptyImpl() : Impl() {
}

CellInterface::Value Cell::EmptyImpl::GetValue(bool) {
    return std::string{};
}

Cell::TextImpl::TextImpl(std::string_view str) :Impl(str) , value_(text_)  {
    if (value_.front() == '\'')
        value_.remove_prefix(1);
}

CellInterface::Value Cell::TextImpl::GetValue(bool) {
    return (std::string)value_;
}

bool IsCycle (const std::vector<Position>& start_nods, Sheet& sheet) {
    std::stack<Position> stack_;
    std::unordered_map<std::string, bool> color_; // - = white; false = green; true = black;
    if (std::find(start_nods.begin(), std::prev(start_nods.end()), start_nods.back())!= std::prev(start_nods.end()))
        return true;
    for (auto pos_ptr = start_nods.rbegin(); pos_ptr != start_nods.rend(); ++pos_ptr )
        stack_.push(*pos_ptr);
    color_[start_nods.back().ToString()] = false;
    while (!stack_.empty()) {
        if (!color_.count(stack_.top().ToString())) {
            color_[stack_.top().ToString()] = false;
            if (sheet.GetCell(stack_.top()) == nullptr)
                continue;
            for (auto& item : sheet.GetCell(stack_.top())->GetReferencedCells()){
                if (sheet.GetCell(item) == nullptr)
                    continue;
                if (!color_.count(item.ToString()))
                    stack_.push(item);
                else if (color_.at(item.ToString()) == false)
                    return true;
            }

        }else {
            if  (color_.at(stack_.top().ToString()) == false)
                color_[stack_.top().ToString()] = true;
            stack_.pop();
        }
    }
    return false;

}

void InvalidatorCash (Position head, Sheet& sheet) {
    std::stack<Position> stack_;
    std::unordered_map<std::string, bool> color_; // - = white; false = green; true = black;

    stack_.push(head);
    while (!stack_.empty()) {
        if (!color_.count(stack_.top().ToString())) {
            color_[stack_.top().ToString()] = false;
//            if (sheet.GetCell(stack_.top())== nullptr)
//                continue;
            for (auto item : dynamic_cast<Cell*>(sheet.GetCell(stack_.top()))->GetParentCells())
                if (!color_.count(item.ToString()))
                    stack_.push(item);

        }else {
            if  (color_[stack_.top().ToString()] == false) {
                color_[stack_.top().ToString()] = true;
//                if (sheet.GetCell(stack_.top())!= nullptr)
                    dynamic_cast<Cell*>(sheet.GetCell(stack_.top()))->is_calculation_ = false;
            }
            stack_.pop();
        }
    }
}

Cell::FormulaImpl::FormulaImpl(std::string_view str, Sheet& sheet, const Position& this_pos, std::vector<Position>& child_cells_) : sheet_(sheet) /*:Impl(str.substr(1)) , value_(text_)*/  {
    str.remove_prefix(1);
    formula_ = ParseFormula((std::string)str);
    auto cilds = formula_->GetReferencedCells();

    cilds.push_back(this_pos);

    if (IsCycle(cilds, sheet_))
        throw CircularDependencyException("Cycle in formula: "s+(std::string)str);

    cilds.pop_back();

    std::swap(child_cells_,cilds);

    for (const Position pos : cilds)
        dynamic_cast<Cell*>(sheet_.GetCell(pos))->DeletePerents(this_pos);



    for (const Position pos : child_cells_){
        if (sheet_.GetCell(pos) == nullptr)
            sheet_.SetCell(pos, {});

        dynamic_cast<Cell*>(sheet_.GetCell(pos))->AddPerents(this_pos);
    }
    if (sheet_.GetCell(this_pos)!= nullptr)
        InvalidatorCash (this_pos, sheet_);

    text_ ='=' + formula_->GetExpression();
    value_ = formula_->Evaluate(sheet_);
}

CellInterface::Value Cell::FormulaImpl::GetValue(bool is_calculation) {
    if (!is_calculation)
        value_ = formula_->Evaluate(sheet_);

    if(std::holds_alternative<FormulaError>(value_))
        return std::get<FormulaError>(value_);
    return std::get<double>(value_);
}
