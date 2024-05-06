#pragma once
#include <vector>

#include "json.h"

namespace json {
// -------------------------------- [JSON Builder] Definition -------------------------
//                                                                                    +
//                                                                                    + -------------------
// ------------------------------------------------------------------------------------ Auxiliary Classes +
    
    class Builder;
    class AfterValueAfterKey;
    class AfterStartDict;
    class AfterStartArray;
    class AfterValueStartArray;

// 
// 
//                                                                                    + -----------
// ------------------------------------------------------------------------------------ After Key +
    class AfterKey {
    public:
        AfterValueAfterKey& Value(Node::Value value);
        AfterStartDict& StartDict();
        AfterStartArray& StartArray();

        void SetBuilder(Builder* builder);

    private:
        Builder* builder_ = nullptr;
    };

// 
// 
//                                                                                    + -------------------------
// ------------------------------------------------------------------------------------ After Value - After Key +

    class AfterValueAfterKey {
    public:
        AfterKey& Key(std::string key);
        Builder& EndDict();

        void SetBuilder(Builder* builder);

    private:
        Builder* builder_ = nullptr;
    };

// 
// 
//                                                                                    + ------------------
// ------------------------------------------------------------------------------------ After Start Dict +

    class AfterStartDict {
    public:
        AfterKey& Key(std::string key);
        Builder& EndDict();

        void SetBuilder(Builder* builder);

    private:
        Builder* builder_ = nullptr;
    };

// 
// 
//                                                                                    + -------------------
// ------------------------------------------------------------------------------------ After Start Array +

    class AfterStartArray {
    public:
        AfterValueStartArray& Value(Node::Value value);
        AfterStartDict& StartDict();
        AfterStartArray& StartArray();
        Builder& EndArray();

        void SetBuilder(Builder* builder);

    private:
        Builder* builder_ = nullptr;
    };

// 
// 
//                                                                                    + ---------------------------------
// ------------------------------------------------------------------------------------ After Value - After Start Array +

    class AfterValueStartArray {
    public:
        AfterValueStartArray& Value(Node::Value value);
        AfterStartDict& StartDict();
        AfterStartArray& StartArray();
        Builder& EndArray();

        void SetBuilder(Builder* builder);

    private:
        Builder* builder_ = nullptr;
    };

// ------------------------------------- Builder Itself -------------------------------
//                                                                                    +
//                                                                                    + ---------
// ------------------------------------------------------------------------------------ Builder +

    struct Afters {
        AfterKey after_key_;
        AfterValueAfterKey after_value_after_key_;
        AfterStartDict after_start_dict_;
        AfterStartArray after_start_array_;
        AfterValueStartArray after_value_start_key_;
    };

    class Builder {
    public:
        Builder();
        Node Build();
        AfterKey& Key(std::string key);
        Builder& Value(Node::Value value);
        AfterStartDict& StartDict();
        AfterStartArray& StartArray();
        Builder& EndDict();
        Builder& EndArray();

        Afters* GetAfters();

    private:
        Node root_;
        std::vector<Node*> nodes_stack_;
        Afters afters_;
        bool afters_are_initialized_ = false;

        Node::Value& GetCurrentValue();
        const Node::Value& GetCurrentValue() const;
        void AssertNewObjectContext() const;
        void AddObject(Node::Value value, bool one_shot); 
        void InitializeAfters();

        // [ one_shot == true ] means that object shouldn't be added to the stack
        // [ one_shot == false ] means the opposite
    };
} // namespace json