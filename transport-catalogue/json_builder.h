#pragma once
#include <vector>

#include "json.h"

namespace json {
// ------------- [JSON Builder] Definition --------------
//                                                      +
//                                                      + -------------------
// ------------------------------------------------------ Auxiliary Classes +
    
    class Builder;
    class KeyProcessor;
    class AfterStartArray;

// 
// 
//                                                      + -----------
// ------------------------------------------------------ After Key +

    class AfterKey {
    public:
        KeyProcessor& Value(Node::Value value);
        KeyProcessor& StartDict();
        AfterStartArray& StartArray();

        void SetBuilder(Builder* builder);

    private:
        Builder* builder_ = nullptr;
    };

// 
// 
//                                                      + -------------------------
// ------------------------------------------------------ After Value - After Key +

    class KeyProcessor {
    public:
        AfterKey& Key(std::string key);
        Builder& EndDict();

        void SetBuilder(Builder* builder);

    private:
        Builder* builder_ = nullptr;
    };

// 
// 
//                                                      + -------------------
// ------------------------------------------------------ After Start Array +

    class AfterStartArray {
    public:
        AfterStartArray& Value(Node::Value value);
        KeyProcessor& StartDict();
        AfterStartArray& StartArray();
        Builder& EndArray();

        void SetBuilder(Builder* builder);

    private:
        Builder* builder_ = nullptr;
    };

// ------------------- Builder Itself -------------------
//                                                      +
//                                                      + ---------
// ------------------------------------------------------ Builder +

    struct Afters {
        AfterKey after_key_;
        KeyProcessor key_processor_;
        AfterStartArray after_start_array_;
    };

    class Builder {
    public:
        Builder();
        Node Build();
        AfterKey& Key(std::string key);
        Builder& Value(Node::Value value);
        KeyProcessor& StartDict();
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