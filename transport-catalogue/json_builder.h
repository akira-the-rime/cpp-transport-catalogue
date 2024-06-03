#pragma once

#include "json.h"

namespace json {
// ----------- [JSON Builder] Definition ------------
//                                                  +
//                                                  + ----------------
// -------------------------------------------------- Builder Itself +

    class Builder final {
    private:
        class AfterStartDict;
        class AfterStartArray;

// 
// 
//                                                  + -----------
// -------------------------------------------------- After Key +

        class AfterKey final {
        public:
            AfterStartDict& Value(Node::Value value);
            AfterStartDict& StartDict();
            AfterStartArray& StartArray();

            void SetBuilder(Builder* builder);

        private:
            Builder* builder_ = nullptr;
        };

// 
// 
//                                                  + ---------------
// -------------------------------------------------- Key Processor +

        class AfterStartDict final {
        public:
            AfterKey& Key(std::string key);
            Builder& EndDict();

            void SetBuilder(Builder* builder);

        private:
            Builder* builder_ = nullptr;
        };

// 
// 
//                                                  + -------------------
// -------------------------------------------------- After Start Array +

        class AfterStartArray final {
        public:
            AfterStartArray& Value(Node::Value value);
            AfterStartDict& StartDict();
            AfterStartArray& StartArray();
            Builder& EndArray();

            void SetBuilder(Builder* builder);

        private:
            Builder* builder_ = nullptr;
        };

// 
// 
//                                                  + ------------------
// -------------------------------------------------- Auxiliary Struct +

        struct Afters final {
            AfterKey after_key_;
            AfterStartDict after_start_dict_;
            AfterStartArray after_start_array_;
        };

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