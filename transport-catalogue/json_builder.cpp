#include "json_builder.h"

namespace json
{
	Node Builder::Build()
	{
		if (nodes_state.size() == 0)
			return node;
		else
			throw std::logic_error("Unfinished structure");
	}
	KeyDictContext Builder::StartDict()
	{
		if (CheckNodesStructure(Call::Start_Dict))
		{
			if (nodes_state.back() == Call::Start_Array)
			{
				Array& arr = std::get<Array>(unfinished_nodes.back()->GetValue());
				arr.push_back(Dict{});
				unfinished_nodes.push_back(&arr.at(arr.size() - 1));
				nodes_state.push_back(Call::Start_Dict);
			}
			else
			{
				*(unfinished_nodes.back()) = Dict{};
				nodes_state.back() = Call::Start_Dict;
			}
		}

		return KeyDictContext(*this);
	}
	BuilderContext Builder::EndDict()
	{
		if (CheckNodesStructure(Call::End_Dict))
		{
			unfinished_nodes.pop_back();
			nodes_state.pop_back();
		}
		return BuilderContext(*this);
	}
	ArrayContext Builder::StartArray()
	{
		if (CheckNodesStructure(Call::Start_Array))
		{
			if (nodes_state.back() == Call::Start_Array)
			{
				Array& arr = std::get<Array>(unfinished_nodes.back()->GetValue());
				arr.push_back(Array{});
				unfinished_nodes.push_back(&arr.at(arr.size() - 1));
				nodes_state.push_back(Call::Start_Array);
			}
			else
			{
				*(unfinished_nodes.back()) = Array{};
				nodes_state.back() = Call::Start_Array;
			}
		}
		return ArrayContext(*this);
	}
	BuilderContext Builder::EndArray()
	{
		if (CheckNodesStructure(Call::End_Array))
		{
			unfinished_nodes.pop_back();
			nodes_state.pop_back();
		}
		return BuilderContext(*this);
	}
	ValueDictContext Builder::Key(const std::string& key)
	{
		if (CheckNodesStructure(Call::Key))
		{
			std::get<Dict>(unfinished_nodes.back()->GetValue()).insert({ key, Node() });
			last_key = key;
			unfinished_nodes.push_back(&(std::get<Dict>(unfinished_nodes.back()->GetValue()).at(key)));
			nodes_state.push_back(Call::Key);
		}
		return ValueDictContext(*this);
	}
	BuilderContext Builder::Value(Node n)
	{
		if (CheckNodesStructure(Call::Value))
		{
			if (nodes_state.back() == Call::Start_Array)
			{
				std::get<Array>(unfinished_nodes.back()->GetValue()).push_back(n);
			}
			else
			{
				*(unfinished_nodes.back()) = n;
				unfinished_nodes.pop_back();
				nodes_state.pop_back();
			}
		}
		return BuilderContext(*this);
	}

	Call Builder::GetLastCall() const
	{
		if (nodes_state.size() == 0)
			return Call::None;
		else
			return nodes_state.back();
	}

	bool Builder::CheckNodesStructure(Call call)
	{
		if (nodes_state.size() == 0)
			throw std::logic_error("Unable to place node here.");

		switch (nodes_state.back())
		{
		case json::None:
			if (call == Call::Value || call == Call::Start_Array || call == Call::Start_Dict)
				return true;
			else
				throw std::logic_error("Can't start with that call");
		case json::Key:
			if (call == Call::Value || call == Call::Start_Array || call == Call::Start_Dict)
				return true;
			else
				throw std::logic_error("Can't start with that call");
		case json::Start_Dict:
			if (call == Call::Key || call == Call::End_Dict)
				return true;
			else
				throw std::logic_error("Dictionary has to start with Key call");
		case json::Start_Array:
			if (call == Call::Value || call == Call::Start_Array || call == Call::End_Array || call == Call::Start_Dict)
				return true;
			else
				throw std::logic_error("Can't start with that call");
		default:
			break;
		}
		throw std::logic_error("Unknowned error");
	}

	Node BuilderContext::Build()
	{
		return builder.Build();
	}

	ArrayContext BuilderContext::StartArray()
	{
		return builder.StartArray();
	}
	KeyDictContext BuilderContext::StartDict()
	{
		return builder.StartDict();
	}
	BuilderContext BuilderContext::EndArray()
	{
		return builder.EndArray();
	}
	BuilderContext BuilderContext::EndDict()
	{
		return builder.EndDict();
	}
	BuilderContext BuilderContext::Value(Node n)
	{
		return builder.Value(n);
	}
	ValueDictContext BuilderContext::Key(const std::string key)
	{
		return builder.Key(key);
	}

	ArrayContext ArrayContext::Value(Node n)
	{
		builder.Value(n);
		return *this;
	}


	KeyDictContext ValueDictContext::Value(Node n)
	{
		builder.Value(n);
		return KeyDictContext(builder);
	}
}