#include "json_builder.h"

namespace json
{

void Builder::StacMoveBack()
{
    auto node_buff = std::move(node_stac_.back());
    node_stac_.pop_back();
    Value(std::move(*node_buff));
}

Builder& Builder::Value(Node value)
{
    if (node_stac_.back()->IsString() && (node_stac_.size() > 2) && (*(node_stac_.end() - 2))->IsDict())
    {
        auto key = std::move(node_stac_.back());
        node_stac_.pop_back();
        node_stac_.back()->AsDict().insert(std::make_pair(std::move(key->AsString()), std::move(value)));
    }
    else if (node_stac_.back()->IsArray())
    {
        node_stac_.back()->AsArray().push_back(std::move(value));
    }
    else
    {
        *node_stac_.back() = std::move(value);
    }

    return *this;
}

Node Builder::Build()
{
    using namespace std::literals;

    if (node_stac_.size() > 1) 
    {
        throw std::logic_error("Document is not ready"s);
    }

    return node_;
}

DictContext Builder::StartDict()
{
    node_stac_.push_back(new Node{Dict{}});
    return DictContext(*this);
}

ArrayContext Builder::StartArray()
{
    node_stac_.push_back(new Node{Array{}});
    return ArrayContext(*this);
}

// --------------------------BaseContext-------------------------------

Builder::BaseContext::BaseContext(Builder& builder): builder_ref_(builder) {}

Builder::BaseContext Builder::BaseContext::Value(Node value)
{
    builder_ref_.Value(std::move(value));
    return *this;
}

Node Builder::BaseContext::Build()
{
    return builder_ref_.Build();
}

DictValuCintext Builder::BaseContext::Key(std::string key)
{
    builder_ref_.node_stac_.push_back(new Node{std::move(key)});
    return DictValuCintext{builder_ref_};
}

DictContext Builder::BaseContext::StartDict()
{
    return builder_ref_.StartDict();
}

ArrayContext Builder::BaseContext::StartArray()
{
    return builder_ref_.StartArray();
}

Builder::BaseContext Builder::BaseContext::EndDict()
{
    builder_ref_.StacMoveBack();
    return *this;
}

Builder::BaseContext Builder::BaseContext::EndArray()
{
    builder_ref_.StacMoveBack();
    return *this;
}

// --------------------------DictContext-------------------------------

DictContext::DictContext(Builder& bulder): BaseContext(bulder) {}

DictValuCintext DictContext::Key(std::string key)
{
    return BaseContext::Key(std::move(key));
}

Builder::BaseContext DictContext::EndDict()
{
    return BaseContext::EndDict();
}

// --------------------------ArrayContext-------------------------------

ArrayContext::ArrayContext(Builder& bulder): BaseContext(bulder) {}

ArrayContext ArrayContext::Value(Node value)
{
    BaseContext::Value(std::move(value));
    return *this;
}

DictContext ArrayContext::StartDict()
{
    return BaseContext::StartDict();
}

ArrayContext ArrayContext::StartArray()
{
    return BaseContext::StartArray();
}

Builder::BaseContext ArrayContext::EndArray()
{
    return BaseContext::EndArray();
}

// --------------------------DictValuCintext-------------------------------

DictValuCintext::DictValuCintext(Builder& bulder): BaseContext(bulder) {}

DictContext DictValuCintext::Value(Node value)
{
    BaseContext::Value(std::move(value));
    return DictContext{builder_ref_};
}

DictContext DictValuCintext::StartDict()
{
    return BaseContext::StartDict();
}

ArrayContext DictValuCintext::StartArray()
{
    return BaseContext::StartArray();
}
} // end namespace json