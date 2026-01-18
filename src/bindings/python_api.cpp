/**
 * @file python_api.cpp
 * @brief Python bindings for TouchdownOS app framework
 */

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include "touchdown/app/app.hpp"
#include "touchdown/core/types.hpp"
#include "lvgl.h"

namespace py = pybind11;

namespace touchdown {
namespace bindings {

/**
 * @brief Python app wrapper
 */
class PyTouchdownApp : public app::TouchdownApp {
public:
    using app::TouchdownApp::TouchdownApp;
    
    // Trampoline methods for virtual functions
    bool init(lv_obj_t* parent) override {
        PYBIND11_OVERRIDE_PURE(bool, app::TouchdownApp, init, parent);
    }
    
    void show() override {
        PYBIND11_OVERRIDE_PURE(void, app::TouchdownApp, show);
    }
    
    void hide() override {
        PYBIND11_OVERRIDE_PURE(void, app::TouchdownApp, hide);
    }
    
    void cleanup() override {
        PYBIND11_OVERRIDE_PURE(void, app::TouchdownApp, cleanup);
    }
    
    void pause() override {
        PYBIND11_OVERRIDE(void, app::TouchdownApp, pause);
    }
    
    void resume() override {
        PYBIND11_OVERRIDE(void, app::TouchdownApp, resume);
    }
    
    void update(uint32_t delta_ms) override {
        PYBIND11_OVERRIDE(void, app::TouchdownApp, update, delta_ms);
    }
    
    bool on_touch(const TouchPoint& point) override {
        PYBIND11_OVERRIDE(bool, app::TouchdownApp, on_touch, point);
    }
    
    bool on_button(const ButtonEvent& event) override {
        PYBIND11_OVERRIDE(bool, app::TouchdownApp, on_button, event);
    }
    
    bool on_back() override {
        PYBIND11_OVERRIDE(bool, app::TouchdownApp, on_back);
    }
};

/**
 * @brief LVGL widget wrappers for Python
 */
class LVGLWidget {
public:
    static intptr_t create_label(intptr_t parent, const std::string& text) {
        lv_obj_t* obj = lv_label_create(reinterpret_cast<lv_obj_t*>(parent));
        lv_label_set_text(obj, text.c_str());
        return reinterpret_cast<intptr_t>(obj);
    }
    
    static intptr_t create_button(intptr_t parent, const std::string& text) {
        lv_obj_t* btn = lv_btn_create(reinterpret_cast<lv_obj_t*>(parent));
        lv_obj_t* label = lv_label_create(btn);
        lv_label_set_text(label, text.c_str());
        lv_obj_center(label);
        return reinterpret_cast<intptr_t>(btn);
    }
    
    static intptr_t create_arc(intptr_t parent) {
        lv_obj_t* arc = lv_arc_create(reinterpret_cast<lv_obj_t*>(parent));
        return reinterpret_cast<intptr_t>(arc);
    }
    
    static intptr_t create_slider(intptr_t parent) {
        lv_obj_t* slider = lv_slider_create(reinterpret_cast<lv_obj_t*>(parent));
        return reinterpret_cast<intptr_t>(slider);
    }
    
    static intptr_t create_list(intptr_t parent) {
        lv_obj_t* list = lv_list_create(reinterpret_cast<lv_obj_t*>(parent));
        return reinterpret_cast<intptr_t>(list);
    }
    
    static void set_text(intptr_t obj, const std::string& text) {
        lv_label_set_text(reinterpret_cast<lv_obj_t*>(obj), text.c_str());
    }
    
    static void set_size(intptr_t obj, int w, int h) {
        lv_obj_set_size(reinterpret_cast<lv_obj_t*>(obj), w, h);
    }
    
    static void set_pos(intptr_t obj, int x, int y) {
        lv_obj_set_pos(reinterpret_cast<lv_obj_t*>(obj), x, y);
    }
    
    static void align(intptr_t obj, int align, int x_ofs, int y_ofs) {
        lv_obj_align(reinterpret_cast<lv_obj_t*>(obj), 
                    static_cast<lv_align_t>(align), x_ofs, y_ofs);
    }
    
    static void set_value(intptr_t obj, int value) {
        // Works for arc, slider, etc.
        lv_obj_t* widget = reinterpret_cast<lv_obj_t*>(obj);
        if (lv_obj_check_type(widget, &lv_slider_class)) {
            lv_slider_set_value(widget, value, LV_ANIM_OFF);
        } else if (lv_obj_check_type(widget, &lv_arc_class)) {
            lv_arc_set_value(widget, value);
        }
    }
    
    static void add_flag(intptr_t obj, int flag) {
        lv_obj_add_flag(reinterpret_cast<lv_obj_t*>(obj), flag);
    }
    
    static void clear_flag(intptr_t obj, int flag) {
        lv_obj_clear_flag(reinterpret_cast<lv_obj_t*>(obj), flag);
    }
    
    static void set_style_bg_color(intptr_t obj, uint32_t color) {
        lv_obj_set_style_bg_color(reinterpret_cast<lv_obj_t*>(obj),
                                  lv_color_hex(color), 0);
    }
    
    static void set_style_text_color(intptr_t obj, uint32_t color) {
        lv_obj_set_style_text_color(reinterpret_cast<lv_obj_t*>(obj),
                                    lv_color_hex(color), 0);
    }
};

PYBIND11_MODULE(touchdown, m) {
    m.doc() = "TouchdownOS Python API";
    
    // Touch event types
    py::enum_<TouchEventType>(m, "TouchEventType")
        .value("PRESS", TouchEventType::PRESS)
        .value("RELEASE", TouchEventType::RELEASE)
        .value("MOVE", TouchEventType::MOVE)
        .value("TAP", TouchEventType::TAP)
        .value("LONG_PRESS", TouchEventType::LONG_PRESS)
        .value("SWIPE_LEFT", TouchEventType::SWIPE_LEFT)
        .value("SWIPE_RIGHT", TouchEventType::SWIPE_RIGHT)
        .value("SWIPE_UP", TouchEventType::SWIPE_UP)
        .value("SWIPE_DOWN", TouchEventType::SWIPE_DOWN);
    
    // Touch point
    py::class_<TouchPoint>(m, "TouchPoint")
        .def(py::init<>())
        .def_readwrite("x", &TouchPoint::x)
        .def_readwrite("y", &TouchPoint::y)
        .def_readwrite("type", &TouchPoint::type)
        .def_readwrite("timestamp_ms", &TouchPoint::timestamp_ms);
    
    // Button event types
    py::enum_<ButtonEventType>(m, "ButtonEventType")
        .value("SINGLE_PRESS", ButtonEventType::SINGLE_PRESS)
        .value("DOUBLE_PRESS", ButtonEventType::DOUBLE_PRESS)
        .value("LONG_PRESS", ButtonEventType::LONG_PRESS)
        .value("RELEASE", ButtonEventType::RELEASE);
    
    // Button event
    py::class_<ButtonEvent>(m, "ButtonEvent")
        .def(py::init<>())
        .def_readwrite("type", &ButtonEvent::type)
        .def_readwrite("timestamp_ms", &ButtonEvent::timestamp_ms)
        .def_readwrite("duration_ms", &ButtonEvent::duration_ms);
    
    // App metadata
    py::class_<app::AppMetadata>(m, "AppMetadata")
        .def(py::init<>())
        .def_readwrite("id", &app::AppMetadata::id)
        .def_readwrite("name", &app::AppMetadata::name)
        .def_readwrite("version", &app::AppMetadata::version)
        .def_readwrite("description", &app::AppMetadata::description)
        .def_readwrite("icon", &app::AppMetadata::icon)
        .def_readwrite("permissions", &app::AppMetadata::permissions);
    
    // TouchdownApp base class
    py::class_<app::TouchdownApp, PyTouchdownApp>(m, "TouchdownApp")
        .def(py::init<const app::AppMetadata&>())
        .def("init", &app::TouchdownApp::init)
        .def("show", &app::TouchdownApp::show)
        .def("hide", &app::TouchdownApp::hide)
        .def("pause", &app::TouchdownApp::pause)
        .def("resume", &app::TouchdownApp::resume)
        .def("update", &app::TouchdownApp::update)
        .def("on_touch", &app::TouchdownApp::on_touch)
        .def("on_button", &app::TouchdownApp::on_button)
        .def("on_back", &app::TouchdownApp::on_back)
        .def("cleanup", &app::TouchdownApp::cleanup)
        .def("get_metadata", &app::TouchdownApp::get_metadata)
        .def("get_container", [](const app::TouchdownApp& self) {
            return reinterpret_cast<intptr_t>(self.get_container());
        })
        .def("is_visible", &app::TouchdownApp::is_visibln)
        .def("request_close", &app::TouchdownApp::request_close);
    
    // LVGL widget helpers
    py::class_<LVGLWidget>(m, "Widget")
        .def_static("create_label", &LVGLWidget::create_label)
        .def_static("create_button", &LVGLWidget::create_button)
        .def_static("create_arc", &LVGLWidget::create_arc)
        .def_static("create_slider", &LVGLWidget::create_slider)
        .def_static("create_list", &LVGLWidget::create_list)
        .def_static("set_text", &LVGLWidget::set_text)
        .def_static("set_size", &LVGLWidget::set_size)
        .def_static("set_pos", &LVGLWidget::set_pos)
        .def_static("align", &LVGLWidget::align)
        .def_static("set_value", &LVGLWidget::set_value)
        .def_static("add_flag", &LVGLWidget::add_flag)
        .def_static("clear_flag", &LVGLWidget::clear_flag)
        .def_static("set_style_bg_color", &LVGLWidget::set_style_bg_color)
        .def_static("set_style_text_color", &LVGLWidget::set_style_text_color);
    
    // LVGL constants
    m.attr("ALIGN_CENTER") = LV_ALIGN_CENTER;
    m.attr("ALIGN_TOP_LEFT") = LV_ALIGN_TOP_LEFT;
    m.attr("ALIGN_TOP_MID") = LV_ALIGN_TOP_MID;
    m.attr("ALIGN_TOP_RIGHT") = LV_ALIGN_TOP_RIGHT;
    m.attr("ALIGN_BOTTOM_LEFT") = LV_ALIGN_BOTTOM_LEFT;
    m.attr("ALIGN_BOTTOM_MID") = LV_ALIGN_BOTTOM_MID;
    m.attr("ALIGN_BOTTOM_RIGHT") = LV_ALIGN_BOTTOM_RIGHT;
    m.attr("ALIGN_LEFT_MID") = LV_ALIGN_LEFT_MID;
    m.attr("ALIGN_RIGHT_MID") = LV_ALIGN_RIGHT_MID;
    
    m.attr("OBJ_FLAG_HIDDEN") = LV_OBJ_FLAG_HIDDEN;
    m.attr("OBJ_FLAG_CLICKABLE") = LV_OBJ_FLAG_CLICKABLE;
    m.attr("OBJ_FLAG_SCROLLABLE") = LV_OBJ_FLAG_SCROLLABLE;
}

} // namespace bindings
} // namespace touchdown
