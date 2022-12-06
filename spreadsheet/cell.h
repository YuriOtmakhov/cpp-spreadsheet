#pragma once

#include "common.h"
#include "formula.h"

#include <memory>
#include <string>
#include <variant>
#include <functional>
#include <vector>
#include <unordered_set>

class Sheet;

class Cell : public CellInterface {
    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;

    struct PositionHasher {
        size_t operator()(const Position& pos) const{
            return std::hash<std::string>{}(pos.ToString());
        }
    };

    std::unique_ptr<Impl> impl_;

    const Position pos_;
    Sheet& sheet_;
    std::vector<Position> child_cells_;
    std::unordered_set<Position, PositionHasher> parant_cells_;

public:
    bool is_calculation_ = false;

    Cell(Sheet& sheet, Position pos);
    ~Cell();

    void Set(std::string text);

    void Clear();

    Value GetValue() const override;

    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override;

    bool IsReferenced() const;

    std::vector<Position> GetParentCells() const;

    void DeletePerents(const Position& cell);
    void AddPerents(const Position& cell);

};
