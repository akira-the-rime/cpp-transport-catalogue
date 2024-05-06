#include <utility>

#include "json_builder.h"

namespace json {
// ------------------------------- [JSON Builder] Realization -------------------------
//                                                                                    +
//                                                                                    + -------------------------------
// ------------------------------------------------------------------------------------ Auxiliary Classes [After Key] +

	AfterValueAfterKey& AfterKey::Value(Node::Value value) {
			builder_->Value(std::move(value));
			return builder_->GetAfters()->after_value_after_key_;
	}

	AfterStartDict& AfterKey::StartDict() {
		builder_->StartDict();
		return builder_->GetAfters()->after_start_dict_;
	}

	AfterStartArray& AfterKey::StartArray() {
		builder_->StartArray();
		return builder_->GetAfters()->after_start_array_;
	}

	void AfterKey::SetBuilder(Builder* builder) {
		builder_ = builder;
	}

// 
// 
//                                                                                    + -------------------------
// ------------------------------------------------------------------------------------ After Value - After Key +

	AfterKey& AfterValueAfterKey::Key(std::string key) {
		builder_->Key(std::move(key));
		return builder_->GetAfters()->after_key_;
	}

	Builder& AfterValueAfterKey::EndDict() {
		builder_->EndDict();
		return *builder_;
	}

	void AfterValueAfterKey::SetBuilder(Builder* builder) {
		builder_ = builder;
	}

// 
// 
//                                                                                    + ------------------
// ------------------------------------------------------------------------------------ After Start Dict +

	AfterKey& AfterStartDict::Key(std::string key) {
		builder_->Key(std::move(key));
		return builder_->GetAfters()->after_key_;
	}

	Builder& AfterStartDict::EndDict() {
		builder_->EndDict();
		return *builder_;
	}

	void AfterStartDict::SetBuilder(Builder* builder) {
		builder_ = builder;
	}
	
// 
// 
//                                                                                    + -------------------
// ------------------------------------------------------------------------------------ After Start Array +

	AfterValueStartArray& AfterStartArray::Value(Node::Value value) {
		builder_->Value(std::move(value));
		return builder_->GetAfters()->after_value_start_key_;
	}

	AfterStartDict& AfterStartArray::StartDict() {
		builder_->StartDict();
		return builder_->GetAfters()->after_start_dict_;
	}

	AfterStartArray& AfterStartArray::StartArray() {
		builder_->StartArray();
		return builder_->GetAfters()->after_start_array_;
	}

	Builder& AfterStartArray::EndArray() {
		builder_->EndArray();
		return *builder_;
	}

	void AfterStartArray::SetBuilder(Builder* builder) {
		builder_ = builder;
	}

// 
// 
//                                                                                    + ---------------------------------
// ------------------------------------------------------------------------------------ After Value - After Start Array +

	AfterValueStartArray& AfterValueStartArray::Value(Node::Value value) {
		builder_->Value(std::move(value));
		return *this;
	}

	AfterStartDict& AfterValueStartArray::StartDict() {
		builder_->StartDict();
		return builder_->GetAfters()->after_start_dict_;
	}

	AfterStartArray& AfterValueStartArray::StartArray() {
		builder_->StartArray();
		return builder_->GetAfters()->after_start_array_;
	}

	Builder& AfterValueStartArray::EndArray() {
		builder_->EndArray();
		return *builder_;
	}

	void AfterValueStartArray::SetBuilder(Builder* builder) {
		builder_ = builder;
	}


// ------------------------------------- Builder Itself -------------------------------
//                                                                                    +
//                                                                                    + -----------------
// ------------------------------------------------------------------------------------ [Key] : [Value] +

	using namespace std::literals;

	AfterKey& Builder::Key(std::string key) {
		Node::Value& host_value = GetCurrentValue();

		if (!std::holds_alternative<Dict>(host_value)) {
			throw std::logic_error("Key() has been called outside a dict"s);
		}

		nodes_stack_.push_back(&std::get<Dict>(host_value)[std::move(key)]);
		return afters_.after_key_;
	}

	Builder& Builder::Value(Node::Value value) {
		AddObject(std::move(value), true);
		return *this;
	}

// 
// 
//                                                                                    + ----------
// ------------------------------------------------------------------------------------ Starters +

	AfterStartDict& Builder::StartDict() {
		if (!afters_are_initialized_) {
			InitializeAfters();
			afters_are_initialized_ = true;
		}

		AddObject(Dict{}, false);
		return afters_.after_start_dict_;
	}

	AfterStartArray& Builder::StartArray() {
		if (!afters_are_initialized_) {
			InitializeAfters();
			afters_are_initialized_ = true;
		}

		AddObject(Array{}, false);
		return afters_.after_start_array_;
	}

// 
// 
//                                                                                    + --------
// ------------------------------------------------------------------------------------ Enders +

	Builder& Builder::EndDict() {
		if (!std::holds_alternative<Dict>(GetCurrentValue())) {
			throw std::logic_error("EndDict() has been called outside a dict"s);
		}

		nodes_stack_.pop_back();
		return *this;
	}

	Builder& Builder::EndArray() {
		if (!std::holds_alternative<Array>(GetCurrentValue())) {
			throw std::logic_error("EndDict() has been called outside an array"s);
		}

		nodes_stack_.pop_back();
		return *this;
	}

// 
// 
//                                                                                    + ------------------
// ------------------------------------------------------------------------------------ Builder & Getter +

	Node Builder::Build() {
		if (!nodes_stack_.empty()) {
			throw std::logic_error("Attempt to build JSON which isn't finalized"s);
		}

		return std::move(root_);
	}

	Afters* Builder::GetAfters() {
		return &afters_;
	}

// 
// 
//                                                                                    + -------------
// ------------------------------------------------------------------------------------ Auxiliaries +

	Builder::Builder()
		: root_()
		, nodes_stack_{ &root_ } {
	}

	// When Dict is in the scope, .Key().Value() or EndDict() are expected
	// When Array is in the scope, .Value() or EndArray() are expected
	// When nullptr (default) is in the scope, first call or dict Value() are expected

	Node::Value& Builder::GetCurrentValue() {
		if (nodes_stack_.empty()) {
			throw std::logic_error("Attempt to change a finalized JSON"s);
		}

		return nodes_stack_.back()->GetValue();
	}

	const Node::Value& Builder::GetCurrentValue() const {
		return const_cast<Builder*>(this)->GetCurrentValue();
	}

	void Builder::AssertNewObjectContext() const {
		if (!std::holds_alternative<std::nullptr_t>(GetCurrentValue())) {
			throw std::logic_error("A new object has been created in the wrong context"s);
		}
	}

	void Builder::AddObject(Node::Value value, bool one_shot) {
		Node::Value& host_value = GetCurrentValue();

		if (std::holds_alternative<Array>(host_value)) {
			Node& node = std::get<Array>(host_value).emplace_back(std::move(value));

			if (!one_shot) {
				nodes_stack_.push_back(&node);
			}
		}
		else {
			AssertNewObjectContext();
			host_value = std::move(value);

			if (one_shot) {
				nodes_stack_.pop_back();
			}
		}
	}

	void Builder::InitializeAfters() {
		afters_.after_key_.SetBuilder(this);
		afters_.after_value_after_key_.SetBuilder(this);
		afters_.after_start_dict_.SetBuilder(this);
		afters_.after_start_array_.SetBuilder(this);
		afters_.after_value_start_key_.SetBuilder(this);
	}
} // namespace json