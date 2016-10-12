// Music 256a / CS 476a | fall 2016
// CCRMA, Stanford University
//
// Author: Mark Rau
// A simple guitar chord player. Keyboard keys a, b, c, d, e, f, g, will determine which chord is played. 
// They are set to G major so the chords are Em, G, Am, Bm, C, and D. 
// A slider is used to pluck the strings which are synthesized using a Karplus-Strong model. 
// Note: The guitar is "tuned" to have a capo on the third fret.
// This was done becauset he string model didn't allow for notes lower than Fs/512. 

#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED



#include "../JuceLibraryCode/JuceHeader.h"
#include "faust/mydsp.h" // Faust module with Karplus Strong
#include <windows.h>
#include <Windows.h>
#include <iostream>
#include <sstream>
#include <atltrace.h>
#include "FaustReverb.h"



//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainContentComponent :
    public AudioAppComponent,
    private Slider::Listener,
    private ToggleButton::Listener,
	private KeyListener
{
public:
    //==============================================================================
    MainContentComponent() : currentSampleRate(0.0)
    {
		//Strumming slider
        addAndMakeVisible (strumSlider);
        strumSlider.setRange (0, 70);
        strumSlider.setValue(0);
        strumSlider.addListener (this);
		strumSlider.setSliderStyle(Slider::LinearBar);
		strumSlider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
		
        
        addAndMakeVisible(strumLabel);
        strumLabel.setText ("Strum", dontSendNotification);
        strumLabel.attachToComponent (&strumSlider, true);
        
        
		//on/off botton
        addAndMakeVisible(onOffButton);
        onOffButton.addListener(this);
        
        addAndMakeVisible(onOffLabel);
        onOffLabel.setText ("On/Off", dontSendNotification);
        onOffLabel.attachToComponent (&onOffButton, true);


        setSize (600, 100);
        
        nChans = 1; // number of output audio channels
        
		//temporary buffer needed to add multiple string signals
		tmpBuffer = new float*[nChans];

        setAudioChannels (0, nChans);
        
        audioBuffer = new float*[nChans];

		// configure keyboard listeners
		getTopLevelComponent()->addKeyListener(this);
    }
	void paint(Graphics& g) override
	{
		g.fillAll(Colours::burlywood);
		g.setColour(Colours::black);
		g.setFont(20.0f);
		g.drawText("Simple Guitar Chords", getLocalBounds(), Justification::centredTop, true);
		g.drawText("Press keys: a, b, c, d, e, g, on keyboard for chords.", getLocalBounds(), Justification::centredBottom, true);
	}
	

    ~MainContentComponent()
    {
        shutdownAudio();
        delete [] audioBuffer;
		delete[] tmpBuffer[0];
		delete[] tmpBuffer;
    }
	

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
        currentSampleRate = sampleRate;
        blockSize = samplesPerBlockExpected;

		tmpBuffer[0] = new float[blockSize];

		//Fill durations buffer
		for (int i = 0; i < 37; ++i) {
			durations[i] = currentSampleRate / frequencies[i];
		}

		//initialize reverb
		reverb.init(sampleRate);
		reverb.buildUserInterface(&reverbControl);
		
        
        ksGtrElow.init(sampleRate); // initializing the Faust module
        ksGtrElow.buildUserInterface(&ksGtrElowControl); // linking the Faust module to the controler

		ksGtrA.init(sampleRate); // initializing the Faust module
		ksGtrA.buildUserInterface(&ksGtrAControl); // linking the Faust module to the controler

		ksGtrD.init(sampleRate); // initializing the Faust module
		ksGtrD.buildUserInterface(&ksGtrDControl); // linking the Faust module to the controler

		ksGtrG.init(sampleRate); // initializing the Faust module
		ksGtrG.buildUserInterface(&ksGtrGControl); // linking the Faust module to the controler

		ksGtrB.init(sampleRate); // initializing the Faust module
		ksGtrB.buildUserInterface(&ksGtrBControl); // linking the Faust module to the controler

		ksGtrEhigh.init(sampleRate); // initializing the Faust module
		ksGtrEhigh.buildUserInterface(&ksGtrEhighControl); // linking the Faust module to the controler
        
        // Print the list of parameters address of "saw"
        // To get the current (default) value of these parameters, sawControl.getParamValue("paramPath") can be used
        for(int i=0; i<ksGtrElowControl.getParamsCount(); i++){
            std::cout << ksGtrElowControl.getParamAdress(i) << "\n";
        }
        
        // setting default values for the Faust module parameters
        ksGtrElowControl.setParamValue("/karplus/resonator/attenuation",0.03);
        ksGtrElowControl.setParamValue("/karplus/excitator/excitation",300);
		ksGtrElowControl.setParamValue("/karplus/resonator/duration", durations[3]);
		ksGtrElowControl.setParamValue("/karplus/level",0 );
		ksGtrElowControl.setParamValue("/karplus/excitator/play", 0);

		ksGtrAControl.setParamValue("/karplus/resonator/attenuation", 0.03);
		ksGtrAControl.setParamValue("/karplus/excitator/excitation", 300);
		ksGtrAControl.setParamValue("/karplus/resonator/duration", durations[7]);
		ksGtrAControl.setParamValue("/karplus/level", 0);
		ksGtrAControl.setParamValue("/karplus/excitator/play", 0);

		ksGtrDControl.setParamValue("/karplus/resonator/attenuation", 0.03);
		ksGtrDControl.setParamValue("/karplus/excitator/excitation", 300);
		ksGtrDControl.setParamValue("/karplus/resonator/duration", durations[10]);
		ksGtrDControl.setParamValue("/karplus/level", 0);
		ksGtrDControl.setParamValue("/karplus/excitator/play", 0);

		ksGtrGControl.setParamValue("/karplus/resonator/attenuation", 0.03);
		ksGtrGControl.setParamValue("/karplus/excitator/excitation", 300);
		ksGtrGControl.setParamValue("/karplus/resonator/duration", durations[12]);
		ksGtrGControl.setParamValue("/karplus/level", 0);
		ksGtrGControl.setParamValue("/karplus/excitator/play", 0);

		ksGtrBControl.setParamValue("/karplus/resonator/attenuation", 0.03);
		ksGtrBControl.setParamValue("/karplus/excitator/excitation", 300);
		ksGtrBControl.setParamValue("/karplus/resonator/duration", durations[22]);
		ksGtrBControl.setParamValue("/karplus/level", 0);
		ksGtrBControl.setParamValue("/karplus/excitator/play", 0);

		ksGtrEhighControl.setParamValue("/karplus/resonator/attenuation", 0.03);
		ksGtrEhighControl.setParamValue("/karplus/excitator/excitation", 300);
		ksGtrEhighControl.setParamValue("/karplus/resonator/duration", durations[27]);
		ksGtrEhighControl.setParamValue("/karplus/level", 0);
		ksGtrEhighControl.setParamValue("/karplus/excitator/play", 0);
    }
    
    // Case where the buffer loop is implemented in Faust (faustSample is not needed in that case)
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        audioBuffer[0] = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);
	
		ksGtrElow.compute(blockSize, NULL, audioBuffer);
		ksGtrA.compute(blockSize, NULL, tmpBuffer);

		for (int sample = 0; sample < bufferToFill.numSamples; ++sample) {
			audioBuffer[0][sample] = audioBuffer[0][sample] + tmpBuffer[0][sample];
		}
		ksGtrD.compute(blockSize, NULL, tmpBuffer);

		for (int sample = 0; sample < bufferToFill.numSamples; ++sample) {
			audioBuffer[0][sample] = audioBuffer[0][sample] + tmpBuffer[0][sample];
		}
		ksGtrG.compute(blockSize, NULL, tmpBuffer);

		for (int sample = 0; sample < bufferToFill.numSamples; ++sample) {
			audioBuffer[0][sample] = audioBuffer[0][sample] + tmpBuffer[0][sample];
		}
		ksGtrB.compute(blockSize, NULL, tmpBuffer);

		for (int sample = 0; sample < bufferToFill.numSamples; ++sample) {
			audioBuffer[0][sample] = audioBuffer[0][sample] + tmpBuffer[0][sample];
		}
		ksGtrEhigh.compute(blockSize, NULL, tmpBuffer);

		for (int sample = 0; sample < bufferToFill.numSamples; ++sample) {
			audioBuffer[0][sample] = audioBuffer[0][sample] + tmpBuffer[0][sample];
		}
		//The reverb kept throwing an error...
		//reverb.compute(bufferToFill.numSamples, audioBuffer, audioBuffer);

    }
    
    void releaseResources() override
    {
        // This will be called when the audio device stops, or when it is being
        // restarted due to a setting change.

        // For more details, see the help for AudioProcessor::releaseResources()
    }

    void resized() override
    {
        const int sliderLeft = 80;
        strumSlider.setBounds (sliderLeft, 150, getWidth() - sliderLeft - 20, 20);
        
        onOffButton.setBounds (250, 50, getWidth() - sliderLeft - 20, 20);
    }
    
    void sliderValueChanged (Slider* slider) override
    {
        if (currentSampleRate > 0.0){
            //Check the strum slider and asign values to mean a pluck on each string
			if (strumSlider.getValue() <12 && strumSlider.getValue() > 8) {
				ksGtrElowControl.setParamValue("/karplus/excitator/play", 1);
			}
			else {
				ksGtrElowControl.setParamValue("/karplus/excitator/play", 0);
			}
			if (strumSlider.getValue() <22 && strumSlider.getValue() > 18) {
				ksGtrAControl.setParamValue("/karplus/excitator/play", 1);
			}
			else {
				ksGtrAControl.setParamValue("/karplus/excitator/play", 0);
			}
			if (strumSlider.getValue() <32 && strumSlider.getValue() > 28) {
				ksGtrDControl.setParamValue("/karplus/excitator/play", 1);
			}
			else {
				ksGtrDControl.setParamValue("/karplus/excitator/play", 0);
			}
			if (strumSlider.getValue() <42 && strumSlider.getValue() > 38) {
				ksGtrGControl.setParamValue("/karplus/excitator/play", 1);
			}
			else {
				ksGtrGControl.setParamValue("/karplus/excitator/play", 0);
			}
			if (strumSlider.getValue() <52 && strumSlider.getValue() > 48) {
				ksGtrBControl.setParamValue("/karplus/excitator/play", 1);
			}
			else {
				ksGtrBControl.setParamValue("/karplus/excitator/play", 0);
			}
			if (strumSlider.getValue() <62 && strumSlider.getValue() > 58) {
				ksGtrEhighControl.setParamValue("/karplus/excitator/play", 1);
			}
			else {
				ksGtrEhighControl.setParamValue("/karplus/excitator/play", 0);
			}
        }
    }
    
    void buttonClicked (Button* button) override
    {
		//set the string levels to 0 if the application is turned off
		if (onOffButton.getToggleState() ==0) {
			ksGtrElowControl.setParamValue("/karplus/level", 0);
			ksGtrAControl.setParamValue("/karplus/level", 0);
			ksGtrDControl.setParamValue("/karplus/level", 0);
			ksGtrGControl.setParamValue("/karplus/level", 0);
			ksGtrBControl.setParamValue("/karplus/level", 0);
			ksGtrEhighControl.setParamValue("/karplus/level", 0);
		}
		else {
			ksGtrElowControl.setParamValue("/karplus/level", 1);
			ksGtrAControl.setParamValue("/karplus/level", 1);
			ksGtrDControl.setParamValue("/karplus/level", 1);
			ksGtrGControl.setParamValue("/karplus/level", 1);
			ksGtrBControl.setParamValue("/karplus/level", 1);
			ksGtrEhighControl.setParamValue("/karplus/level", 1);
		}
    }
	bool keyPressed(const KeyPress &key, Component* /* originatingComponent */) override
	{
		//Check if keyboard keys are pressed for chords
			if (key.getTextCharacter() == 'a') {
				ksGtrAControl.setParamValue("/karplus/level", 1);
				ksGtrElowControl.setParamValue("/karplus/level", 0);
				ksGtrAControl.setParamValue("/karplus/resonator/duration", durations[5 + capo]);
				ksGtrDControl.setParamValue("/karplus/resonator/duration", durations[12 + capo]);
				ksGtrGControl.setParamValue("/karplus/resonator/duration", durations[17 + capo]);
				ksGtrBControl.setParamValue("/karplus/resonator/duration", durations[20 + capo]);
				ksGtrEhighControl.setParamValue("/karplus/resonator/duration", durations[24 + capo]);
			}
			if (key.getTextCharacter() == 'g') {
				ksGtrElowControl.setParamValue("/karplus/level", 1);
				ksGtrAControl.setParamValue("/karplus/level", 1);
				ksGtrElowControl.setParamValue("/karplus/resonator/duration", durations[3]);
				ksGtrAControl.setParamValue("/karplus/resonator/duration", durations[7]);
				ksGtrDControl.setParamValue("/karplus/resonator/duration", durations[10]);
				ksGtrGControl.setParamValue("/karplus/resonator/duration", durations[15]);
				ksGtrBControl.setParamValue("/karplus/resonator/duration", durations[22]);
				ksGtrEhighControl.setParamValue("/karplus/resonator/duration", durations[27]);
			}
			if (key.getTextCharacter() == 'e') {
				ksGtrElowControl.setParamValue("/karplus/level", 1);
				ksGtrAControl.setParamValue("/karplus/level", 1);
				ksGtrElowControl.setParamValue("/karplus/resonator/duration", durations[0 + capo]);
				ksGtrAControl.setParamValue("/karplus/resonator/duration", durations[7 + capo]);
				ksGtrDControl.setParamValue("/karplus/resonator/duration", durations[12 + capo]);
				ksGtrGControl.setParamValue("/karplus/resonator/duration", durations[15 + capo]);
				ksGtrBControl.setParamValue("/karplus/resonator/duration", durations[19 + capo]);
				ksGtrEhighControl.setParamValue("/karplus/resonator/duration", durations[24 + capo]);
			}
			if (key.getTextCharacter() == 'b') {
				ksGtrAControl.setParamValue("/karplus/level", 1);
				ksGtrElowControl.setParamValue("/karplus/level", 0);
				ksGtrAControl.setParamValue("/karplus/resonator/duration", durations[7 + capo]);
				ksGtrDControl.setParamValue("/karplus/resonator/duration", durations[14 + capo]);
				ksGtrGControl.setParamValue("/karplus/resonator/duration", durations[19 + capo]);
				ksGtrBControl.setParamValue("/karplus/resonator/duration", durations[22 + capo]);
				ksGtrEhighControl.setParamValue("/karplus/resonator/duration", durations[26 + capo]);
			}
			if (key.getTextCharacter() == 'a') {
				ksGtrAControl.setParamValue("/karplus/level", 1);
				ksGtrElowControl.setParamValue("/karplus/level", 0);
				ksGtrAControl.setParamValue("/karplus/resonator/duration", durations[5 + capo]);
				ksGtrDControl.setParamValue("/karplus/resonator/duration", durations[12 + capo]);
				ksGtrGControl.setParamValue("/karplus/resonator/duration", durations[17 + capo]);
				ksGtrBControl.setParamValue("/karplus/resonator/duration", durations[20 + capo]);
				ksGtrEhighControl.setParamValue("/karplus/resonator/duration", durations[24 + capo]);
			}
			if (key.getTextCharacter() == 'c') {
				ksGtrAControl.setParamValue("/karplus/level", 1);
				ksGtrElowControl.setParamValue("/karplus/level", 0);
				ksGtrAControl.setParamValue("/karplus/resonator/duration", durations[8 + capo]);
				ksGtrDControl.setParamValue("/karplus/resonator/duration", durations[12 + capo]);
				ksGtrGControl.setParamValue("/karplus/resonator/duration", durations[15 + capo]);
				ksGtrBControl.setParamValue("/karplus/resonator/duration", durations[20 + capo]);
				ksGtrEhighControl.setParamValue("/karplus/resonator/duration", durations[24 + capo]);
			}
			if (key.getTextCharacter() == 'd') {
				ksGtrElowControl.setParamValue("/karplus/level", 0);
				ksGtrAControl.setParamValue("/karplus/level", 0);
				ksGtrDControl.setParamValue("/karplus/resonator/duration", durations[10 + capo]);
				ksGtrGControl.setParamValue("/karplus/resonator/duration", durations[17 + capo]);
				ksGtrBControl.setParamValue("/karplus/resonator/duration", durations[22 + capo]);
				ksGtrEhighControl.setParamValue("/karplus/resonator/duration", durations[26 + capo]);
			}
		return true;
	}

	
private:
	//lowEstring
    mydsp ksGtrElow; // the Faust module (mydsp.h)
    MapUI ksGtrElowControl; // used to easily control the Faust module (mydsp.h)
	//A string
	mydsp ksGtrA; // the Faust module (mydsp.h)
	MapUI ksGtrAControl; // used to easily control the Faust module (mydsp.h)
	//D string
	mydsp ksGtrD; // the Faust module (mydsp.h)
	MapUI ksGtrDControl; // used to easily control the Faust module (mydsp.h)
	//G string
	mydsp ksGtrG; // the Faust module (mydsp.h)
	MapUI ksGtrGControl; // used to easily control the Faust module (mydsp.h)
	//B string
	mydsp ksGtrB; // the Faust module (mydsp.h)
	MapUI ksGtrBControl; // used to easily control the Faust module (mydsp.h)
	//highE string
	mydsp ksGtrEhigh; // the Faust module (mydsp.h)
	MapUI ksGtrEhighControl; // used to easily control the Faust module (mydsp.h)

	//Faust Reverb
	FaustReverb reverb;
	MapUI reverbControl;

    
    Slider strumSlider;
    Slider gainSlider;
    ToggleButton onOffButton;
	TextButton eChord;
	TextButton gChord;
	TextButton aChord;
	TextButton bChord;
	TextButton cChord;
	TextButton dChord;
    
    Label strumLabel;
    Label gainLabel;
    Label onOffLabel;
	Label eChordLabel;
	Label gChordLabel;
	Label aChordLabel;
	Label bChordLabel;
	Label cChordLabel;
	Label dChordLabel;



    int blockSize, nChans;
    double currentSampleRate;

	//Define frequency values
	double frequencies[37] = {82.41,87.31,92.50,98.00,103.83,110.0,116.54,123.47,130.81,138.59,146.83,155.56,164.81,174.61,185.00,196.00,207.65,220.00,233.08,246.94,261.63,277.18,293.66,311.13,329.63,349.23,369.99,392.00,415.30,440.00,466.16,493.88,523.25,554.37,587.33,622.25,659.25};
	//hold the durations for the Karplus-Strong Module
	float durations[37]; 

	//Used to shift all chords to a higher pitch
	int capo = 3;

    
	float** tmpBuffer;

    float** audioBuffer; // multichannel audio buffer used both for input and output

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent()     { return new MainContentComponent(); }


#endif  // MAINCOMPONENT_H_INCLUDED
