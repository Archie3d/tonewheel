#include <gtest/gtest.h>
#include "engine/core/list.h"

using namespace tonewheel::core;

#if 0

// The following should not compile, since the item does not inherit ListItem<>
struct DataInvalid
{
    int value;
};

List<DataInvalid> invalidList;

#endif

struct Data : public ListItem<Data>
{
    int value;
};

constexpr int SIZE{ 16 };

/** Test creating and iterating a list. */
TEST(core, List)
{
    Data data[SIZE];

    for (int i = 0; i < SIZE; ++i)
        data[i].value = i;

    List<Data> list_a;
    List<Data> list_b;

    EXPECT_TRUE(list_a.isEmpty());
    EXPECT_TRUE(list_b.isEmpty());

    // Move all the items into the list_a
    for (auto& d : data)
        list_a.append(&d);

    EXPECT_FALSE(list_a.isEmpty());

    // Iterate the items
    auto* it = list_a.first();
    int counter{ 0 };

    while (it != nullptr) {
        EXPECT_EQ(it->value, counter);
        ++counter;
        it = it->next();
    }

    EXPECT_EQ(counter, SIZE);

    // Access by index (from head and from tail)
    for (int i = 0; i < SIZE; ++i) {
        EXPECT_EQ(data[i].value, list_a[i]->value);
        EXPECT_EQ(data[i].value, list_a[-SIZE + i]->value);
    }

    // Removing items
    list_a.remove(list_a.first());
    list_a.remove(list_a.last());

    for (int i = 1; i < SIZE - 1; ++i)
        EXPECT_EQ(data[i].value, list_a[i - 1]->value);

    // Inserting items back
    list_a.prepend(&data[0]);
    list_a.append(&data[SIZE - 1]);

    for (int i = 0; i < SIZE; ++i)
        EXPECT_EQ(data[i].value, list_a[i]->value);

    // Move every other value to the list_b
    it = list_a.first();

    while (it != nullptr) {
        if (it->value % 2 == 0) {
            auto* item = it;
            it = list_a.removeAndReturnNext(it);
            list_b.append(item);
        } else {
            it = it->next();
        }
    }

    // Iterate both lists
    auto* it_a{ list_a.first() };
    auto* it_b{ list_b.first() };

    for (int i = 0; i < SIZE; ++i) {
        if (data[i].value % 2 == 0) {
            EXPECT_EQ(data[i].value, it_b->value);
            it_b = it_b->next();
        } else {
            EXPECT_EQ(data[i].value, it_a->value);
            it_a = it_a->next();
        }
    }
}
