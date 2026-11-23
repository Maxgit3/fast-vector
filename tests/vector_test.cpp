#include <gtest/gtest.h>
#include <string>
#include "fast_ds/custom_vector.hpp"


namespace {
    struct Tracker {
        static inline int constructions = 0;
        static inline int moves = 0;
        static inline int copies = 0;
        static inline int destructions = 0;

        int value = 0;

        explicit Tracker(int v) : value(v) { ++constructions; }
        Tracker(const Tracker& other) : value(other.value) { ++copies; }
        Tracker(Tracker&& other) noexcept : value(other.value) { 
            other.value = -1;
            ++moves; 
        }
        ~Tracker() { ++destructions; }

        static void reset() {
            constructions = 0;
            moves = 0;
            copies = 0;
            destructions = 0;
        }
    };
} // namespace

TEST(CustomVectorTest, StartsEmpty) {
    fast_ds::CustomVector<int> vec;
    EXPECT_EQ(vec.size(), 0);
    EXPECT_EQ(vec.capacity(), 0);
    EXPECT_TRUE(vec.empty());
}

TEST(CustomVectorTest, PushBackAndGrowth) {
    fast_ds::CustomVector<int> vec;
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i * 10);
    }
    EXPECT_EQ(vec.size(), 10);
    EXPECT_EQ(vec.capacity(), 16);
    EXPECT_EQ(vec[0], 0);
    EXPECT_EQ(vec[5], 50);
    EXPECT_EQ(vec.at(9), 90);
    EXPECT_THROW(vec.at(10), std::out_of_range);
}

TEST(CustomVectorTest, HeapAllocatedTypesCleanUpProperly) {
    fast_ds::CustomVector<std::string> vec;
    vec.emplace_back("First long string exceeding SSO Buffer");
    vec.emplace_back("Second long string exceeding SSO Buffer");
    vec.emplace_back("Third long string exceeding SSO Buffer");
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec[1], "Second long string exceeding SSO Buffer");
    vec.pop_back();
    EXPECT_EQ(vec.size(), 2);
}

TEST(CustomVectorTest, MoveSemanticsAndDestructionCount) {
    Tracker::reset();
    {
        fast_ds::CustomVector<Tracker> vec;
        vec.emplace_back(1); // cap 0 -> 1 (1 construction)
        vec.emplace_back(2); // cap 1 -> 2 (1 construction, 1 move, 1 destruction of old slot)
        vec.emplace_back(3); // cap 2 -> 4 (1 construction, 2 moves, 2 destructions of old slots)
        EXPECT_EQ(Tracker::constructions, 3);
        EXPECT_EQ(Tracker::copies, 0);
        EXPECT_EQ(Tracker::moves, 3);
    } // vec goes out of scope, 3 active elements should be destroyed

    // Every constructed or moved object should be destroyed exactly once
    EXPECT_EQ(Tracker::constructions + Tracker::moves + Tracker::copies, Tracker::destructions);
}