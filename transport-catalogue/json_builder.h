#pragma once

#include "json.h"

namespace json
{
	class DictItemContext;
	class KeyItemContext;
	class ArrayItemContext;

	class Builder
	{
	private:
		Node node_;
		std::vector<Node*> node_stack_;
		bool last_is_key_ = false;
	public:
		Builder();
		Node& Build(void);
		Builder& Value(const Node& value);
		DictItemContext StartDict(void);
		Builder& Key(const std::string& key);
		Builder& EndDict(void);
		ArrayItemContext StartArray(void);
		Builder& EndArray(void);

		
	};


	class DictItemContext {
	public:
		DictItemContext(Builder& builder);
		KeyItemContext Key(const std::string& key);
		Builder& EndDict();

	private:
		Builder& builder_;
	};

	class KeyItemContext
	{
	public:
		KeyItemContext(Builder& builder);
		ArrayItemContext StartArray(void);
		DictItemContext StartDict(void);
		DictItemContext Value(const Node& value);
	private:
		Builder& builder_;
	};

	class ArrayItemContext
	{
	public:
		ArrayItemContext(Builder& builder);
		ArrayItemContext Value(const Node& value);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		Builder& EndArray(void);

	private:
		Builder& builder_;
	};
} // namespace json