#include "engine.h"
#include "conversion.h"
#include "application.h"
#include "js_object.h"
#include "js_array.h"

VALUE rbqml_cEngine;

typedef struct {
    qmlbind_engine engine;
} engine_t;

void engine_free(void *p) {
    engine_t *data = (engine_t *)p;
    qmlbind_engine_release(data->engine);
    xfree(data);
}

static const rb_data_type_t data_type = {
    "QML::Engine",
    { NULL, &engine_free }
};

qmlbind_engine rbqml_get_engine(VALUE self) {
    engine_t *data;
    TypedData_Get_Struct(self, void, &data_type, data);
    return data->engine;
}

static VALUE engine_alloc(VALUE klass) {
    engine_t *data = ALLOC(engine_t);
    data->engine = NULL;
    return TypedData_Wrap_Struct(klass, &data_type, data);
}

static VALUE engine_initialize(VALUE self) {
    if (NIL_P(rbqml_application_instance)) {
        rb_raise(rb_eRuntimeError, "QML::Application not initialized");
    }

    engine_t *data;
    TypedData_Get_Struct(self, void, &data_type, data);
    data->engine = qmlbind_engine_new();
    return self;
}

static VALUE engine_evaluate(VALUE self, int argc, VALUE *argv) {
    qmlbind_engine engine = rbqml_get_engine(self);

    VALUE str, file, lineNum;

    rb_scan_args(argc, argv, "12", &str, &file, &lineNum);

    qmlbind_value value =  qmlbind_engine_eval(engine, rb_string_value_cstr(&str),
                                               RTEST(file) ? rb_string_value_cstr(&file) : "",
                                               RTEST(lineNum) ? rb_num2int(lineNum) : 1);

    return rbqml_to_ruby(value, self);
}

static VALUE engine_build_array(VALUE self, VALUE len) {
    qmlbind_engine engine = rbqml_get_engine(self);

    qmlbind_value array = qmlbind_engine_new_array(engine, NUM2INT(len));
    VALUE value = rbqml_js_object_new(rbqml_cJSArray, array, self);
    qmlbind_value_release(array);

    return value;
}

static VALUE engine_build_object(VALUE self) {
    qmlbind_engine engine = rbqml_get_engine(self);

    qmlbind_value obj = qmlbind_engine_new_object(engine);
    VALUE value = rbqml_js_object_new(rbqml_cJSObject, obj, self);
    qmlbind_value_release(obj);

    return value;
}

void rbqml_init_engine() {
    rbqml_cEngine = rb_define_class_under(rb_path2class("QML"), "Engine", rb_cObject);
    rb_define_alloc_func(rbqml_cEngine, &engine_alloc);

    rb_define_private_method(rbqml_cEngine, "initialize", &engine_initialize, 0);
    rb_define_method(rbqml_cEngine, "evaluate", &engine_evaluate, -1);
    rb_define_private_method(rbqml_cEngine, "build_array", &engine_build_array, 1);
    rb_define_private_method(rbqml_cEngine, "build_object", &engine_build_object, 0);
}