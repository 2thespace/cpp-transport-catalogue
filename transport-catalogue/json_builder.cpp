#include "json_builder.h"

namespace json
{
	Builder::Builder()
	{
		node_stack_.emplace_back(&node_);
	}
	Node& Builder::Build(void)
	{
		if (node_.IsNull() || node_stack_.size() > 1) {
			throw std::logic_error("Empty node");
		}
		return node_;
	}

	Builder& Builder::Value(const Node& value)
	{
		if (node_stack_.empty()) {
			throw std::logic_error("uncorrect key");
		}
		auto* node = node_stack_.back();
		if (node->IsArray()) {
			auto& back_value = std::get<Array>(node->GetNonConstValue());
			back_value.push_back(value);
		}
		else if(node_.IsNull() || last_is_key_ ) {
			*node = value;
			node_stack_.pop_back();
			last_is_key_ = false;
		}
		else  {
			throw std::logic_error("Uncorrect value");
		}

		
		return *this;
	}

	DictItemContext Builder::StartDict(void)
	{
		if (node_stack_.empty()) {
			throw std::logic_error("uncorrect key");
		}
		auto* node = node_stack_.back();
		if(node->IsArray()) {
			auto& back_value = std::get<Array>(node->GetNonConstValue());
			back_value.push_back(Node(Dict()));
			node_stack_.push_back(&back_value.back());
		}
		else if (node->IsNull()) {
			node->GetNonConstValue() = Dict();
		}
		else {
			throw std::logic_error("uncorrect dict");
		}
		
		return *this;
	}

	Builder& Builder::Key(const std::string& key)
	{	
		if (node_stack_.empty()) {
			throw std::logic_error("uncorrect key");
		}
		if (!node_stack_.back()->IsDict()) {
			throw std::logic_error("uncorrect key");
		}
		else {
			auto* node = node_stack_.back();
			auto& back_value = std::get<Dict>(node->GetNonConstValue())[key];
			node_stack_.emplace_back(&back_value);
			last_is_key_ = true;
		}

		return *this;
	}

	Builder& Builder::EndDict(void)
	{
		if (node_stack_.empty()) {
			throw std::logic_error("uncorrect key");
		}
		auto* node = node_stack_.back();
		if (!node->IsDict()) {
			throw std::logic_error("Uncorrect end dict"); 
		}
		node_stack_.pop_back();
		return *this;
	}

	ArrayItemContext Builder::StartArray(void)
	{
		if (node_stack_.empty()) {
			throw std::logic_error("uncorrect key");
		}
		auto* node = node_stack_.back();
		if (node->IsArray()) {
			auto& back_value = std::get<Array>(node->GetNonConstValue());
			back_value.push_back(Node(Array()));
			node_stack_.push_back(&back_value.back());
		}
		else if (node->IsNull()) {
			node->GetNonConstValue() = Array();
		}
		else {
			throw std::logic_error("uncorrect array");
		}
		
		return *this;
	}

	Builder& Builder::EndArray(void)
	{
		if (node_stack_.empty()) {
			throw std::logic_error("uncorrect key");
		}
		auto* node = node_stack_.back();
		if (!node->IsArray()) {
			throw std::logic_error("Uncorrect end array");
		}
		node_stack_.pop_back();
		return *this;
	}

	DictItemContext::DictItemContext(Builder& builder)
		: builder_(builder)
	{}
	
	KeyItemContext DictItemContext::Key(const std::string& key)
	{
		return builder_.Key(key);
	}

	Builder& DictItemContext::EndDict(void)
	{
		return builder_.EndDict();
	}

	KeyItemContext::KeyItemContext(Builder& builder): builder_(builder)
	{	}

	DictItemContext KeyItemContext::StartDict(void)
	{
		return builder_.StartDict();
	}
	ArrayItemContext KeyItemContext::StartArray(void)
	{
		return builder_.StartArray();
	}
	
	DictItemContext KeyItemContext::Value(const Node& value)
	{
		return builder_.Value(value);
	}

	ArrayItemContext::ArrayItemContext(Builder& builder):builder_(builder) {}

	ArrayItemContext ArrayItemContext::Value(const Node& value)
	{
		return builder_.Value(value);
	}

	DictItemContext ArrayItemContext::StartDict()
	{
		return builder_.StartDict();
	}
	ArrayItemContext ArrayItemContext::StartArray()
	{
		return builder_.StartArray();
	}

	Builder& ArrayItemContext::EndArray(void)
	{
		return builder_.EndArray();
	}
} // namespace json
