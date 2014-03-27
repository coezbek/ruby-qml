require 'spec_helper'

describe QML::MetaType do

  before do
    @metatype = QML::MetaType::Q_STRING
  end

  describe '#id' do
    it 'returns the meta type id' do
      expect(@metatype.id).to eq(10)
    end
  end

  describe '#name, #to_s' do
    it 'returns the type name' do
      expect(@metatype.name).to eq(:QString)
      expect(@metatype.to_s).to eq('QString')
    end
  end

  describe '#inspect' do
    it 'returns the detail representation of the meta type' do
      expect(@metatype.inspect).to eq('<QML::MetaType:QString>')
    end
  end

  describe '#==, #eql?' do
    it 'compares 2 meta types' do
      other = @metatype.dup
      expect(@metatype == other).to be(true)
      expect(@metatype.eql?(other)).to be(true)
    end
  end

  describe '#hash' do
    it 'returns the hash value acquired from #id' do
      expect(@metatype.hash).to eq(@metatype.id.hash)
    end
  end

  describe 'ruby_class' do
    it 'returns the corresponding Ruby class' do
      expect(QML::MetaType::Q_STRING.ruby_class).to be(String)
      expect(QML::MetaType::INT.ruby_class).to be(Integer)
    end
  end

end