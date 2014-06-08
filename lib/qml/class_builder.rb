require 'logger'
require 'ropework'
require 'qml/qt_object_base'
require 'qml/qml'

module QML

  class UnsupportedTypeValue
    def inspect
      "<unsupported type>"
    end
    alias_method :to_s, :inspect
  end

  class QtProperty
    prepend Ropework::Bindable

    attr_reader :changed

    def initialize(metaobj, objptr, name)
      super()
      @metaobj = metaobj
      @objptr = objptr
      @name = name
      @changed = QtSignal.new(metaobj, objptr, @metaobj.notify_signal(@name))
    end

    def value=(newval)
      @metaobj.set_property(@objptr, @name, newval)
    end

    def value
      @metaobj.get_property(@objptr, @name) rescue UnsupportedTypeValue.new
    end
  end

  class QtSignal < Ropework::Signal
    def initialize(metaobj, objptr, name)
      super(variadic: true)
      @objptr = objptr
      @metaobj = metaobj
      @name = name
    end

    private

    def connected(block)
      return if @initialized
      @metaobj.connect_signal(@objptr, @name, method(:emit))
      @initialized = true
    end
  end

  class ClassBuilder

    attr_reader :metaobj, :klass

    def initialize(metaobj, klass)
      @metaobj = metaobj
      @klass = klass
    end

    def build
      create unless @klass
      return if @klass.meta_object == @metaobj

      @metaobj.method_names.each do |name|
        define_method(name)
      end
      @metaobj.property_names.each do |name|
        define_property(name)
      end
      @metaobj.enumerators.each do |k, v|
        @klass.send :const_set, k, v
      end
      @klass.send :meta_object=, @metaobj

      self
    end

    private

    def create
      super_metaobj = @metaobj.super_class
      @klass = Class.new(super_metaobj ? super_metaobj.build_class : QtObjectBase)
    end

    def define_method(name)
      metaobj = @metaobj
      return if @metaobj.private?(name)
      if @metaobj.signal?(name)
        @klass.send :variadic_signal, name, factory: proc { |obj|
          QtSignal.new(@metaobj, obj.object_pointer, name)
        }
      else
        @klass.send :define_method, name do |*args|
          metaobj.invoke_method(@object_pointer, name, args)
        end
        @klass.send :private, name if @metaobj.protected?(name)
      end
    end

    def define_property(name)
      metaobj = @metaobj
      @klass.send :property, name, factory: proc { |obj|
        QtProperty.new(@metaobj, obj.object_pointer, name)
      }
    end
  end
end
