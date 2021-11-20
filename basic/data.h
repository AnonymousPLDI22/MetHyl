//
// Created by pro on 2021/8/23.
//

#ifndef DPSYNTHESISNEW_DATA_H
#define DPSYNTHESISNEW_DATA_H

#include "value.h"

class Semantics;
class ListValue;
class BTreeValue;

class Data {
    void setValue(Value* _value) {
        value = _value;
        if (_value) value->addRef();
    }
public:
    Value* value;
    Type* getType() const {return value ? value->getType() : TVOID;}
    Data(Value* _value) {setValue(_value);}
    Data(): value(nullptr) {}
    Data(int _value) {setValue(new IntValue(_value));}
    Data(const Data& _data) {setValue(_data.value);}
    Data(Data&& data) {setValue(data.value);}
    Data& operator = (Data&& data) noexcept {
        if (data.value == value) return *this;
        if (value) value->delRef();
        setValue(data.value);
        return *this;
    }
    Data& operator = (const Data& data) noexcept {
        if (data.value == value) return *this;
        if (value) value->delRef();
        setValue(data.value);
        return *this;
    }
    ~Data() {
        if (value) value->delRef();
        value = nullptr;
    }
    std::string toString() const {
        return value ? value->toString() : "empty";
    }
    ListValue* getList() const;
    BTreeValue* getBTree() const;
    int getInt() const;
    int getBool() const;
    int isNull() const {return !value;}
    Data accessProd(int ind) const;
    std::vector<Data> getProdContents() const;
    Semantics* getSemantics() const;
};

bool operator != (const Data& d1, const Data& d2);
bool operator == (const Data& d1, const Data& d2);
bool operator <= (const Data& d1, const Data& d2);

typedef std::vector<Data> DataList;
typedef std::vector<DataList> DataStorage;

typedef DataList Example;
typedef std::pair<DataList, Data> PointExample;

namespace data {
    extern std::string dataList2String(const DataList& data_list);
    DataStorage cartesianProduct(const DataStorage& separate_data);
    DataList unfoldProdData(const Data& d);
    DataList mergeDataList(const DataList& x, const DataList& y);
}

#endif //DPSYNTHESISNEW_DATA_H
