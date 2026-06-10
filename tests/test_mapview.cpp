#include "TestFramework.hpp"
#include "ui/MapView.hpp"

TEST_CASE(mapview_neighbor_at_returns_minus_one_for_empty_space) {
    MapView m;
    CHECK_EQ(m.neighborAt({0.f, 0.f}), -1);
    CHECK_EQ(m.neighborAt({1200.f, 700.f}), -1);
}

TEST_CASE(mapview_neighbor_at_returns_index_when_clicked_close) {
    MapView m;
    // Northland centro (340, 220) — clic justo ahi devuelve 0
    CHECK_EQ(m.neighborAt({340.f, 220.f}), 0);
    // Easteria (920, 220)
    CHECK_EQ(m.neighborAt({920.f, 220.f}), 1);
    // Southaven (624, 600)
    CHECK_EQ(m.neighborAt({624.f, 600.f}), 2);
}

TEST_CASE(mapview_update_does_not_crash) {
    MapView m;
    m.update(0.016f);
    m.update(0.016f);
    CHECK(true);
}
