/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SmartPedalAudioProcessorEditor::SmartPedalAudioProcessorEditor (SmartPedalAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to

    // Overall Widgets
    addAndMakeVisible(loadButton);
    loadButton.setButtonText("LOAD MODEL");
    loadButton.addListener(this);

    addAndMakeVisible(modelLabel);
    modelLabel.setText("Model", juce::NotificationType::dontSendNotification);
    modelLabel.setJustificationType(juce::Justification::centred);
    modelLabel.setColour(juce::Label::textColourId, juce::Colours::black);

    addAndMakeVisible(modelSelect);
    modelSelect.setColour(juce::Label::textColourId, juce::Colours::black);
    int c = 1;
    for (const auto& jsonFile : processor.jsonFiles) {
        modelSelect.addItem(jsonFile.getFileName(), c);
        c += 1;
    }
    modelSelect.onChange = [this] {modelSelectChanged();};

    auto font = modelLabel.getFont();
    float height = font.getHeight();
    font.setHeight(height);
    modelLabel.setFont(font);

    // Set Widget Graphics
    blackHexKnobLAF.setLookAndFeel(ImageCache::getFromMemory(BinaryData::knob_hex_png, BinaryData::knob_hex_pngSize));

    // Pre Amp Pedal Widgets
 
    // Overdrive
    odFootSw.setImages(true, true, true,
        ImageCache::getFromMemory(BinaryData::footswitch_up_png, BinaryData::footswitch_up_pngSize), 1.0, Colours::transparentWhite,
        Image(), 1.0, Colours::transparentWhite,
        ImageCache::getFromMemory(BinaryData::footswitch_down_png, BinaryData::footswitch_down_pngSize), 1.0, Colours::transparentWhite,
        0.0);
    addAndMakeVisible(odFootSw);
    odFootSw.addListener(this);

    odLED.setImages(true, true, true,
        ImageCache::getFromMemory(BinaryData::led_red_on_png, BinaryData::led_red_on_pngSize), 1.0, Colours::transparentWhite,
        Image(), 1.0, Colours::transparentWhite,
        ImageCache::getFromMemory(BinaryData::led_red_on_png, BinaryData::led_red_on_pngSize), 1.0, Colours::transparentWhite,
        0.0);
    addAndMakeVisible(odLED);

    driveSliderAttach = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(processor.treeState, GAIN_ID, odDriveKnob);
    addAndMakeVisible(odDriveKnob);
    odDriveKnob.setLookAndFeel(&blackHexKnobLAF);
    odDriveKnob.addListener(this);
    //odDriveKnob.setRange(0.0, 1.0);
    //odDriveKnob.setValue(processor.pedalDriveKnobState);
    odDriveKnob.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    odDriveKnob.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, false, 50, 20);
    //odDriveKnob.setNumDecimalPlacesToDisplay(1);
    odDriveKnob.setDoubleClickReturnValue(true, 0.5);

    masterSliderAttach = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(processor.treeState, MASTER_ID, odLevelKnob);
    addAndMakeVisible(odLevelKnob);
    odLevelKnob.setLookAndFeel(&blackHexKnobLAF);
    odLevelKnob.addListener(this);
    //odLevelKnob.setRange(-23.0, 25.0);
    //odLevelKnob.setValue(processor.pedalLevelKnobState);
    odLevelKnob.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    odLevelKnob.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, false, 50, 20);
    //odLevelKnob.setNumDecimalPlacesToDisplay(1);
    odLevelKnob.setDoubleClickReturnValue(true, 0.5);

    // Size of plugin GUI
    setSize (500, 650);
    resetImages();
}

SmartPedalAudioProcessorEditor::~SmartPedalAudioProcessorEditor()
{
}

//==============================================================================
void SmartPedalAudioProcessorEditor::paint (Graphics& g)
{
    // Workaround for graphics on Windows builds (clipping code doesn't work correctly on Windows)
    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    g.drawImageAt(background, 0, 0);  // Debug Line: Redraw entire background image
    #else
    // Redraw only the clipped part of the background image
    juce::Rectangle<int> ClipRect = g.getClipBounds(); 
    g.drawImage(background, ClipRect.getX(), ClipRect.getY(), ClipRect.getWidth(), ClipRect.getHeight(), ClipRect.getX(), ClipRect.getY(), ClipRect.getWidth(), ClipRect.getHeight());
    #endif
}

void SmartPedalAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    //Overall Widgets
    loadButton.setBounds(184, 68, 120, 20);
    modelSelect.setBounds(142, 36, 210, 25);
    modelLabel.setBounds(193, 12, 90, 25);

    // Overdrive Widgets
    odDriveKnob.setBounds(112, 115, 125, 145);
    odLevelKnob.setBounds(283, 115, 125, 145);
    odFootSw.setBounds(220, 459, 75, 105);
    odLED.setBounds(234, 398, 75, 105);
}


void SmartPedalAudioProcessorEditor::loadButtonClicked()
{
    myChooser = std::make_unique<FileChooser> ("Select a folder to load models from",
                                               File::getSpecialLocation (File::userDesktopDirectory),
                                               "*.json");
 
    auto folderChooserFlags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectDirectories | FileBrowserComponent::canSelectFiles;
 
    myChooser->launchAsync (folderChooserFlags, [this] (const FileChooser& chooser)                
    {
        Array<File> files;
        if (chooser.getResult().existsAsFile()) { // If a file is selected
            files = chooser.getResult().getParentDirectory().findChildFiles(2, false, "*.wav");
        } else if (chooser.getResult().isDirectory()){ // Else folder is selected
            files = chooser.getResult().findChildFiles(2, false, "*.wav");
        }
        
        // Change the target IR folder
        //processor.userAppDataDirectory_irs = chooser.getResult();
        processor.jsonFiles.clear();
        //processor.num_irs = 0;
        modelSelect.clear();

        if (files.size() > 0) {
            //Array<File> files = chooser.getResults();
            for (auto file : files) {

                //File fullpath = processor.userAppDataDirectory_irs.getFullPathName() + "/" + file.getFileName();
                //bool b = fullpath.existsAsFile();
                //bool b = file.existsAsFile();

                //if (b == false) {

                    // Copy selected file to model directory and load into dropdown menu
                    // bool a = file.copyFileTo(fullpath);
                    // if (a == true) {

                    // Add to ir-a menu
                modelSelect.addItem(file.getFileNameWithoutExtension(), processor.jsonFiles.size() + 1);

                processor.jsonFiles.push_back(file);
                processor.num_models += 1;

                //}
                // Sort jsonFiles alphabetically
                //std::sort(processor.irFiles.begin(), processor.irFiles.end());
            //}
            }
            // Load last file in selected files list for both A and B IR's
            //modelSelect.setSelectedItemIndex(processor.jsonFiles.size(), juce::NotificationType::dontSendNotification);
            //processor.loadIRa(files.getLast());
            //processor.loadIRb(files.getLast());
            modelSelect.setSelectedItemIndex(0, juce::NotificationType::dontSendNotification);
            
            modelSelectChanged();

        }
    });
    
}

/*
void SmartPedalAudioProcessorEditor::loadButtonClicked()
{
    FileChooser chooser("Load a .json model...",
        {},
        "*.json");
    if (chooser.browseForFileToOpen())
    {
        File file = chooser.getResult();
        processor.loadConfig(file);
    }
}
*/

void SmartPedalAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    if (button == &odFootSw) {
        odFootSwClicked();
    } else if (button == &loadButton) {
        loadButtonClicked();
    }
}

void SmartPedalAudioProcessorEditor::odFootSwClicked() {
    if (processor.fw_state == 0)
        processor.fw_state = 1;
    else
        processor.fw_state = 0;
    repaint();
}

void SmartPedalAudioProcessorEditor::sliderValueChanged(Slider* slider)
{
    // Overdrive    
    //if (slider == &odDriveKnob)
    //    processor.set_odDrive(slider->getValue());
    //else if (slider == &odLevelKnob)
    //    processor.set_odLevel(slider->getValue());
}

void SmartPedalAudioProcessorEditor::modelSelectChanged()
{
    const int selectedFileIndex = modelSelect.getSelectedItemIndex();
    if (selectedFileIndex >= 0 && selectedFileIndex < processor.jsonFiles.size()) {
        processor.loadConfig(processor.jsonFiles[selectedFileIndex]);
    }
}


void SmartPedalAudioProcessorEditor::resetImages()
{
    if (processor.fw_state == 0) {
        odFootSw.setImages(true, true, true,
            ImageCache::getFromMemory(BinaryData::footswitch_up_png, BinaryData::footswitch_up_pngSize), 1.0, Colours::transparentWhite,
            Image(), 1.0, Colours::transparentWhite,
            ImageCache::getFromMemory(BinaryData::footswitch_up_png, BinaryData::footswitch_up_pngSize), 1.0, Colours::transparentWhite,
            0.0);
        odLED.setImages(true, true, true,
            ImageCache::getFromMemory(BinaryData::led_red_off_png, BinaryData::led_red_off_pngSize), 1.0, Colours::transparentWhite,
            Image(), 1.0, Colours::transparentWhite,
            ImageCache::getFromMemory(BinaryData::led_red_off_png, BinaryData::led_red_off_pngSize), 1.0, Colours::transparentWhite,
            0.0);
    }
    else {
        odFootSw.setImages(true, true, true,
            ImageCache::getFromMemory(BinaryData::footswitch_down_png, BinaryData::footswitch_down_pngSize), 1.0, Colours::transparentWhite,
            Image(), 1.0, Colours::transparentWhite,
            ImageCache::getFromMemory(BinaryData::footswitch_down_png, BinaryData::footswitch_down_pngSize), 1.0, Colours::transparentWhite,
            0.0);
       odLED.setImages(true, true, true,
            ImageCache::getFromMemory(BinaryData::led_red_on_png, BinaryData::led_red_on_pngSize), 1.0, Colours::transparentWhite,
            Image(), 1.0, Colours::transparentWhite,
            ImageCache::getFromMemory(BinaryData::led_red_on_png, BinaryData::led_red_on_pngSize), 1.0, Colours::transparentWhite,
            0.0);
    }
}