#pragma once
#include <memory>

#include "json.h"

namespace json
{

class DictContext;
class ArrayContext;
class DictValuCintext;

class Builder
{
protected:

    Node node_;
    std::vector<Node*> node_stac_;

    void StacMoveBack();
public:

    class BaseContext
    {
    protected:
        Builder& builder_ref_;
    public:
        BaseContext(Builder& builder);

        BaseContext Value(Node value);
        Node Build();
        DictValuCintext Key(std::string key);
        DictContext StartDict();
        ArrayContext StartArray();
        BaseContext EndDict();
        BaseContext EndArray();
    }; // end class BaseContext

    Builder()
    {
        node_stac_.push_back(&node_);
    }

    Builder& Value(Node value);
    Node Build();
    DictContext StartDict();
    ArrayContext StartArray(); 
}; // end class Builder

class DictContext: public Builder::BaseContext
{
public:
    DictContext(Builder& bulder);

    BaseContext Value(Node value) = delete;
    Node Build() = delete;
    BaseContext EndArray() = delete;
    DictContext StartDict() = delete;
    ArrayContext StartArray() = delete;

    DictValuCintext Key(std::string key);
    BaseContext EndDict();
}; // end class DictContext

class ArrayContext: public Builder::BaseContext
{
public:
    ArrayContext(Builder& bulder);

    Node Build() = delete;
    DictValuCintext Key(std::string key) = delete;
    BaseContext EndDict() = delete;

    ArrayContext Value(Node value);
    DictContext StartDict();
    ArrayContext StartArray();
    BaseContext EndArray();

}; // end class ArrayContext

class DictValuCintext: public Builder::BaseContext
{
public: 
    DictValuCintext(Builder& bulder);


    Node Build() = delete;
    BaseContext EndArray() = delete;
    DictValuCintext Key(std::string key) = delete;
    BaseContext EndDict() = delete;

    DictContext Value(Node value);
    DictContext StartDict();
    ArrayContext StartArray();
}; // end class DictValuCintextext
} // end namespace jsom


