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

#ifndef NWBFORMAT_H
#define NWBFORMAT_H

#include "HDF5FileFormat.h"

namespace NWBRecording
{

	struct NWBRecordingInfo
	{
		String sourceName;
		float bitVolts;
		int processorId;
		int sourceId;
		int nChannels;
		int nSamplesPerSpike;
		float sampleRate;
		String spikeElectrodeName;
	};

	class NWBFile : public HDF5FileBase
	{
	public:
		NWBFile(String fName, String ver); //with whatever arguments it's necessary
		~NWBFile();
		bool startNewRecording(int recordingNumber, const Array<NWBRecordingInfo>& continuousArray, const Array<NWBRecordingInfo>& electrodeArray);
		void stopRecording();
		void writeData(int datasetID, int channel, int nSamples, const int16* data);
		void writeTimestamps(int datasetID, int nSamples, const double* data);
		void writeSpike(int electrodeId, const uint16* data, uint64 timestamp);
		void writeTTLEvent(int channel, int id, uint8 source, uint64 timestamp);
		void writeMessage(const char* msg, uint64 timestamp);
		String getFileName() override;

	protected:
		int createFileStructure() override;

	private:
		HDF5RecordingData* createRecordingStructures(String basePath, NWBRecordingInfo& info, String helpText, int chunk_size);

		const String filename;
		const String GUIVersion;

		OwnedArray<HDF5RecordingData> continuousDataSets;
		OwnedArray<HDF5RecordingData> continuousDataSetsTS;
		StringArray continuousBasePaths;
		Array<uint64> numContinuousSamples;
		Array<NWBRecordingInfo> continuousInfoStructs;
		OwnedArray<HDF5RecordingData> spikeDataSets;
		OwnedArray<HDF5RecordingData> spikeDataSetsTS;
		StringArray spikeBasePaths;
		Array<uint64> numSpikes;
		Array<NWBRecordingInfo> spikeInfoStructs;
		ScopedPointer<HDF5RecordingData> eventsDataSet;
		ScopedPointer<HDF5RecordingData> eventsDataSetTS;
		String eventsBasePath;
		uint64 numEvents;
		ScopedPointer<HDF5RecordingData> messagesDataSet;
		ScopedPointer<HDF5RecordingData> messagesDataSetTS;
		String messagesBasePath;
		uint64 numMessages;

		ScopedPointer<HDF5RecordingData> eventsControlDataSet;

		HeapBlock<int16> transformBlock;
		int spikeMaxSize;

		//whatever is needed
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NWBFile);

	};

}

#endif