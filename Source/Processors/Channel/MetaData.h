/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2014 Open Ephys

------------------------------------------------------------------

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef METADATA_H_INCLUDED
#define METADATA_H_INCLUDED

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../PluginManager/OpenEphysPlugin.h"

class GenericProcessor;
class MetaDataEvent;

/**
Metadata Objects
Any extra info a processor wants to define that can't fit inside the standard fields can be defined as metadata.
A metadata value can be of any type of the listed in MetaDataTypes and with any dimension. For example,
a metadata value of type INTEGER and size 3 would be an array of 3 integers. For strings, simply set
a type of CHAR and a value equal to the maximum length of the string plus one, for the null char.
*/

class PLUGIN_API MetaDataDescriptor
	: public ReferenceCountedObject
{
public:
	enum MetaDataTypes
	{
		CHAR,
		INT8,
		UINT8,
		INT16,
		UINT16,
		INT32,
		UINT32,
		INT64,
		UINT64,
		FLOAT,
		DOUBLE
	};

	MetaDataDescriptor(MetaDataTypes type, unsigned int length, String name, String desc);
	~MetaDataDescriptor();
	MetaDataTypes getType() const;
	unsigned int getLength() const;
	size_t getDataSize() const;
	String getName() const;
	String getDescription() const;

	bool isEqual(const MetaDataDescriptor& other) const;
	bool operator==(const MetaDataDescriptor& other) const;

	static size_t getTypeSize(MetaDataTypes type);
private:
	MetaDataDescriptor() = delete;
	String m_name;
	String m_desc;
	MetaDataTypes m_type;
	unsigned int m_length;

	JUCE_LEAK_DETECTOR(MetaDataDescriptor);
};

class PLUGIN_API MetaDataValue
	: public ReferenceCountedObject
{
	friend MetaDataEvent; //The Serialize method must access the raw data pointer.
public:
	MetaDataValue(MetaDataDescriptor::MetaDataTypes type, unsigned int length);
	MetaDataValue(const MetaDataDescriptor& desc);
	//To be able to set value at object creation
	MetaDataValue(MetaDataDescriptor::MetaDataTypes type, unsigned int length, const void* data);
	MetaDataValue(const MetaDataDescriptor& desc, const void* data);

	bool isOfType(const MetaDataDescriptor& desc) const;
	bool isOfType(const MetaDataDescriptor* desc) const;

	MetaDataDescriptor::MetaDataTypes getDataType() const;
	unsigned int getDataLength() const;
	size_t getDataSize() const;

	MetaDataValue(const MetaDataValue&);
	MetaDataValue& operator= (const MetaDataValue&);
#if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
	MetaDataValue& operator= (MetaDataValue&&);
#endif
	~MetaDataValue();

	//Get-set for strings
	void setValue(const String& data);
	void getValue(String& data) const;

	//Get-set for single-size types. Defined only for the types present on MetaDataTypes
	template <typename T>
	void setValue(T data);

	template <typename T>
	void getValue(T& data) const;

	//Get set for arrays, both in raw form and in juce array form. Defined only for the types present on MetaDataTypes
	template <typename T>
	void setValue(const T* data);

	template <typename T>
	void getValue(T* data) const;

	template <typename T>
	void setValue(const Array<T>& data);

	template <typename T>
	void getValue(Array<T>& data) const;

private:
	MetaDataValue() = delete;
	void allocSpace();
	HeapBlock<char> m_data;
	MetaDataDescriptor::MetaDataTypes m_type;
	unsigned int m_length;
	size_t m_size;

	JUCE_LEAK_DETECTOR(MetaDataValue);
};

typedef ReferenceCountedArray<MetaDataDescriptor,CriticalSection> MetaDataDescriptorArray;
typedef ReferenceCountedArray<MetaDataValue,CriticalSection> MetaDataValueArray;
typedef ReferenceCountedObjectPtr<MetaDataDescriptor> MetaDataDescriptorPtr;
typedef ReferenceCountedObjectPtr<MetaDataValue> MetaDataValuePtr;

//Inherited for all info objects that have metadata
class PLUGIN_API MetaDataInfoObject
{
protected:
	MetaDataInfoObject();
public:
	void addMetaData(MetaDataDescriptor* desc, MetaDataValue* val);
	const MetaDataDescriptor* getMetaDataDescriptor(int index) const;
	const MetaDataValue* getMetaDataValue(int index) const;
	const int getMetaDataCount() const;
protected:
	MetaDataDescriptorArray m_metaDataDescriptorArray;
	MetaDataValueArray m_metaDataValueArray;
};

class MetaDataEventLock
{
	//GenericProcessor will set this to true when copying channels in the update method so no other processor but the one which
	//created the object can call addEventMetaData. This is done this way because since the events themselves are created by the
	//source processot and not modified, changing the metadata information will cause errors when trying to decode the data embedded
	//in the event itself.
	friend class GenericProcessor;
protected:
	bool eventMetaDataLock{ false };
	MetaDataEventLock();
};

//Special class for event and spike info objects, whose events can hold extra metadata
class PLUGIN_API MetaDataEventObject : private MetaDataEventLock
{
public:
	//This method will only work when creating the info object, but not for those copied down the chain
	void addEventMetaData(MetaDataDescriptor* desc);
	const MetaDataDescriptor* getEventMetaDataDescriptor(int index) const;
	size_t getTotalEventMetaDataSize() const;
	const int getEventMetaDataCount() const;
protected:
	MetaDataDescriptorArray m_eventMetaDataDescriptorArray;
	MetaDataEventObject();
	size_t m_totalSize{ 0 };
};

//And the base from which event objects can hold their metadata before serializing
class PLUGIN_API MetaDataEvent
{
protected:
	void serializeMetaData(void* dstBuffer) const;
	bool deserializeMetaData(const MetaDataEventObject* info, const void* srcBuffer, int size);
	MetaDataEvent();
	MetaDataValueArray m_metaDataValues;
};


#endif