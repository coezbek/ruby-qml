#include "engine.h"
#include "conversion.h"
#include "application.h"
#include "js_object.h"
#include "js_array.h"

VALUE rbqml_cEngine;
static VALUE cQMLError;

typedef struct {
    qmlbind_engine engine;
} engine_t;

static void engine_free(void *p) {
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
    TypedData_Get_Struct(self, engine_t, &data_type, data);
    return data->engine;
}

static VALUE engine_alloc(VALUE klass) {
    engine_t *data = ALLOC(engine_t);
    data->engine = NULL;
    return TypedData_Wrap_Struct(klass, &data_type, data);
}

static VALUE engine_initialize(VALUE self) {
    engine_t *data;
    TypedData_Get_Struct(self, engine_t, &data_type, data);
    data->engine = qmlbind_engine_new();
    return self;
}

/*
 * Adds a QML import path to the {Engine}.
 * @param path [String]
 * @see http://doc.qt.io/qt-5/qtqml-syntax-imports.html#qml-import-path
 */
static VALUE engine_add_import_path(VALUE self, VALUE path) {
    qmlbind_engine engine = rbqml_get_engine(self);
    path = rb_funcall(path, rb_intern("to_s"), 0);
    qmlbind_engine_add_import_path(engine, rb_string_value_cstr(&path));
    return self;
}

static VALUE engine_evaluate(int argc, VALUE *argv, VALUE self) {
    qmlbind_engine engine = rbqml_get_engine(self);

    VALUE str, file, lineNum;

    rb_scan_args(argc, argv, "12", &str, &file, &lineNum);

    qmlbind_value value =  qmlbind_engine_eval(engine, rb_string_value_cstr(&str),
                                               RTEST(file) ? rb_string_value_cstr(&file) : "",
                                               RTEST(lineNum) ? rb_num2int(lineNum) : 1);

    if (qmlbind_value_is_error(value)) {
        qmlbind_string error = qmlbind_value_get_string(value);
        VALUE errorStr = rb_str_new(qmlbind_string_get_chars(error), qmlbind_string_get_length(error));
        qmlbind_string_release(error);

        qmlbind_value_release(value);

        rb_exc_raise(rb_funcall(cQMLError, rb_intern("new"), 1, errorStr));
    }

    VALUE result = rbqml_to_ruby(value);
    qmlbind_value_release(value);
    return result;
}

static VALUE engine_new_array(VALUE self, VALUE len) {
    qmlbind_engine engine = rbqml_get_engine(self);

    qmlbind_value array = qmlbind_engine_new_array(engine, NUM2INT(len));
    VALUE value = rbqml_js_object_new(rbqml_cJSArray, array);
    qmlbind_value_release(array);

    return value;
}

static VALUE engine_new_object(VALUE self) {
    qmlbind_engine engine = rbqml_get_engine(self);

    qmlbind_value obj = qmlbind_engine_new_object(engine);
    VALUE value = rbqml_js_object_new(rbqml_cJSObject, obj);
    qmlbind_value_release(obj);

    return value;
}

void rbqml_init_engine() {
    rb_require("qml/errors");
    cQMLError = rb_path2class("QML::QMLError");

    rbqml_cEngine = rb_define_class_under(rb_path2class("QML"), "Engine", rb_cObject);
    rb_define_alloc_func(rbqml_cEngine, &engine_alloc);

    rb_define_private_method(rbqml_cEngine, "initialize", &engine_initialize, 0);
    rb_define_method(rbqml_cEngine, "add_import_path", &engine_add_import_path, 1);
    rb_define_method(rbqml_cEngine, "evaluate", &engine_evaluate, -1);
    rb_define_method(rbqml_cEngine, "new_array", &engine_new_array, 1);
    rb_define_method(rbqml_cEngine, "new_object", &engine_new_object, 0);
}
