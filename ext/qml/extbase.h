#pragma once
#include "util.h"
#include "rubyvalue.h"

#define METHOD_TYPE_NAME(name) decltype(name), name

namespace RubyQml {

enum class MethodAccess
{
    Public,
    Protected,
    Private
};

template <typename TDerived>
class ExtBase
{
public:

    RubyValue self() { return mSelf; }

    static TDerived *getPointer(RubyValue value)
    {
        auto klass = rubyClass();
        protect([&] {
            if (!RTEST(rb_obj_is_kind_of(value, klass))) {
                rb_raise(rb_eTypeError, "expected %s, got %s", rb_class2name(klass), rb_obj_classname(value));
            }
        });
        TDerived *ptr;
        Data_Get_Struct(VALUE(value), TDerived, ptr);
        return ptr;
    }

    static RubyValue newAsRuby()
    {
        auto klass = rubyClass();
        RubyValue ret;
        protect([&] {
            ret = rb_obj_alloc(klass);
        });
        return ret;
    }

    class ClassBuilder;

    static RubyValue rubyClass()
    {
        return mKlass;
    }

private:

    static void markImpl(TDerived *ptr) noexcept
    {
        ptr->mark();
    }

    static void dealloc(TDerived *ptr) noexcept
    {
        withoutGvl([&] {
            ptr->~TDerived();
        });
        ruby_xfree(ptr);
    }

    static VALUE alloc(VALUE klass) noexcept
    {
        auto ptr = ruby_xmalloc(sizeof(TDerived));
        new(ptr) TDerived();
        auto self = Data_Wrap_Struct(klass, &markImpl, &dealloc, ptr);
        static_cast<TDerived *>(ptr)->mSelf = self;
        return self;
    }

    RubyValue mSelf;
    static RubyValue mKlass;
};

template <typename TDerived>
RubyValue ExtBase<TDerived>::mKlass = Qnil;

template <typename TDerived>
class ExtBase<TDerived>::ClassBuilder
{
public:

    ClassBuilder(const char *outerPath, const char *name)
    {
        protect([&] {
            mKlass = rb_define_class_under(rb_path2class(outerPath), name, rb_cObject);
            rb_define_alloc_func(mKlass, &alloc);
        });
    }

    RubyValue rubyClass() { return mKlass; }

    template <typename TMemberFunction, TMemberFunction memfn>
    ClassBuilder &defineMethod(const char *name, MethodAccess access = MethodAccess::Public)
    {
        using wrapper = MethodWrapper<TMemberFunction, memfn>;
        auto func = (VALUE (*)(...))&wrapper::apply;
        auto argc = wrapper::argc;

        protect([&] {
            switch (access) {
            case MethodAccess::Public:
                rb_define_method(mKlass, name, func, argc);
                break;
            case MethodAccess::Protected:
                rb_define_protected_method(mKlass, name, func, argc);
                break;
            case MethodAccess::Private:
                rb_define_private_method(mKlass, name, func, argc);
                break;
            }
        });
        return *this;
    }

    ClassBuilder &aliasMethod(const char *newName, const char *oldName)
    {
        protect([&] {
            rb_alias(mKlass, rb_intern(newName), rb_intern(oldName));
        });
        return *this;
    }

private:

    template <typename T>
    struct ToVALUE
    {
        using type = VALUE;
    };

    template <typename TMemberFunction, TMemberFunction memfn>
    struct MethodWrapper;

    template <typename ... TArgs, RubyValue (TDerived::*memfn)(TArgs ...)>
    struct MethodWrapper<RubyValue (TDerived::*)(TArgs ...), memfn>
    {
        static constexpr size_t argc = sizeof...(TArgs);

        static VALUE apply(RubyValue self, typename ToVALUE<TArgs>::type ... args)
        {
            RubyValue ret;
            unprotect([&] {
                ret = (getPointer(self)->*memfn)(args ...);
            });
            return ret;
        }
    };

    template <typename ... TArgs, RubyValue (TDerived::*memfn)(TArgs ...) const>
    struct MethodWrapper<RubyValue (TDerived::*)(TArgs ...) const, memfn>
    {
        static constexpr size_t argc = sizeof...(TArgs);

        static VALUE apply(RubyValue self, typename ToVALUE<TArgs>::type ... args)
        {
            RubyValue ret;
            unprotect([&] {
                ret = (getPointer(self)->*memfn)(args ...);
            });
            return ret;
        }
    };
};

}
