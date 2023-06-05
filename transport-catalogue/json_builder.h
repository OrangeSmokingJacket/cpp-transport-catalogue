#pragma once

#include "json.h"
#include <deque>

namespace json
{
	class BuilderContext;
	class ArrayContext;
	class KeyDictContext;
	class ValueDictContext;

	enum Call { None, Key, Value, Start_Dict, End_Dict, Start_Array, End_Array };
	class Builder
	{
	public:
		Builder()
		{
			unfinished_nodes.push_back(&node);
			nodes_state.push_back(Call::None);
		}
		Node Build();
		KeyDictContext StartDict();
		BuilderContext EndDict();
		ArrayContext StartArray();
		BuilderContext EndArray();
		ValueDictContext Key(const std::string& key);
		BuilderContext Value(Node n);

		Call GetLastCall() const;

	private:
		bool CheckNodesStructure(Call call);

		std::string last_key = "";
		Node node;
		std::deque<Node*> unfinished_nodes;
		std::deque<Call> nodes_state;
	};

	class BuilderContext
	{
	public:
		BuilderContext(Builder& b) : builder(b) {}
		Node Build();
		ArrayContext StartArray();
		KeyDictContext StartDict();
		BuilderContext EndArray();
		BuilderContext EndDict();
		BuilderContext Value(Node n);
		ValueDictContext Key(const std::string key);
	protected:
		Builder& builder;
	};
	class ArrayContext : public BuilderContext
	{
	public:
		ArrayContext(Builder& b) : BuilderContext(b) {}
		ArrayContext Value(Node n);
	private:
		using BuilderContext::Build;
		using BuilderContext::EndDict;
		using BuilderContext::Key;
	};
	class KeyDictContext : public BuilderContext
	{
	public:
		KeyDictContext(Builder& b) : BuilderContext(b) {}

	private:
		using BuilderContext::Build;
		using BuilderContext::StartDict;
		using BuilderContext::StartArray;
		using BuilderContext::EndArray;
		using BuilderContext::Value;
	};
	class ValueDictContext : public BuilderContext
	{
	public:
		ValueDictContext(Builder& b) : BuilderContext(b) {}
		KeyDictContext Value(Node n);

	private:
		using BuilderContext::Build;
		using BuilderContext::EndArray;
		using BuilderContext::EndDict;
		using BuilderContext::Key;
	};
}