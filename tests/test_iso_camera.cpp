#include "TestFramework.hpp"
#include "ui/IsoCamera.hpp"

TEST_CASE(iso_camera_origin_maps_to_viewport) {
    IsoCamera cam;
    cam.setViewport(100.f, 100.f);
    cam.setCenter(0.f, 0.f);
    auto s = cam.worldToScreen(0.f, 0.f, 0.f);
    CHECK_NEAR(s.x, 100.f, 0.01);
    CHECK_NEAR(s.y, 100.f, 0.01);
}

TEST_CASE(iso_camera_round_trip) {
    IsoCamera cam;
    cam.setViewport(500.f, 300.f);
    cam.setCenter(5.f, 5.f);
    cam.setZoom(1.5f);
    auto s = cam.worldToScreen(8.f, 3.f, 0.f);
    auto w = cam.screenToWorld(s.x, s.y);
    CHECK_NEAR(w.x, 8.f, 0.01);
    CHECK_NEAR(w.y, 3.f, 0.01);
}

TEST_CASE(iso_camera_zoom_clamps) {
    IsoCamera cam;
    cam.setZoom(0.1f);
    CHECK_NEAR(cam.zoom(), 0.5f, 0.001);
    cam.setZoom(5.f);
    CHECK_NEAR(cam.zoom(), 2.f, 0.001);
    cam.setZoom(1.2f);
    CHECK_NEAR(cam.zoom(), 1.2f, 0.001);
}

TEST_CASE(iso_camera_height_pushes_up) {
    IsoCamera cam;
    cam.setViewport(0.f, 0.f);
    cam.setCenter(0.f, 0.f);
    cam.setZoom(1.f);
    auto sGround = cam.worldToScreen(2.f, 2.f, 0.f);
    auto sHigh   = cam.worldToScreen(2.f, 2.f, 3.f);
    // z mayor -> y de pantalla menor (mas arriba)
    CHECK(sHigh.y < sGround.y);
}

TEST_CASE(iso_camera_x_axis_goes_right_and_down) {
    IsoCamera cam;
    cam.setViewport(0.f, 0.f);
    cam.setCenter(0.f, 0.f);
    cam.setZoom(1.f);
    auto s = cam.worldToScreen(1.f, 0.f, 0.f);
    // x positivo -> derecha y abajo en isometric
    CHECK(s.x > 0.f);
    CHECK(s.y > 0.f);
}
