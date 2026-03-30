#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "parameters/ParameterIDs.h"

// ── Constructor / Destructor ───────────────────────────────────────────────────

RC20PluginProcessor::RC20PluginProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput ("Input",  juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "RC20CloneState", createParameterLayout())
{
    // Build the fixed signal chain in order.
    modules_[0] = std::make_unique<NoiseModule>();
    modules_[1] = std::make_unique<WobbleModule>();
    modules_[2] = std::make_unique<DistortionModule>();
    modules_[3] = std::make_unique<SpaceModule>();
    modules_[4] = std::make_unique<MagicModule>();
    modules_[5] = std::make_unique<LimitModule>();

    // Cache global parameter pointers.
    driftParam_       = apvts.getRawParameterValue(ParameterIDs::drift);
    outputLevelParam_ = apvts.getRawParameterValue(ParameterIDs::output_level);

    // Let each module cache its own parameter pointers (message thread, safe).
    for (auto& module : modules_)
        module->prepareParameters(apvts);
}

// ── AudioProcessor ─────────────────────────────────────────────────────────────

void RC20PluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    const juce::dsp::ProcessSpec spec {
        sampleRate,
        static_cast<juce::uint32>(samplesPerBlock),
        static_cast<juce::uint32>(getTotalNumOutputChannels())
    };

    driftGenerator_.prepare(spec);

    for (auto& module : modules_)
        module->prepare(spec);
}

void RC20PluginProcessor::releaseResources()
{
    for (auto& module : modules_)
        module->reset();
}

bool RC20PluginProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Stereo in / stereo out only.
    return layouts.getMainInputChannelSet()  == juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void RC20PluginProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                       juce::MidiBuffer&          midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    // Read global drift depth and advance the generator one block.
    const float driftDepth = *driftParam_;
    const float driftValue = driftGenerator_.process(driftDepth);

    // Wrap the buffer in an AudioBlock for zero-copy module processing.
    juce::dsp::AudioBlock<float> block(buffer);
    ProcessContext ctx { block, driftValue };

    for (auto& module : modules_)
        module->process(ctx);

    // Apply output level (dB -> linear).
    const float gainLinear = juce::Decibels::decibelsToGain(*outputLevelParam_);
    block.multiplyBy(gainLinear);
}

// ── Editor ─────────────────────────────────────────────────────────────────────

juce::AudioProcessorEditor* RC20PluginProcessor::createEditor()
{
    return new RC20PluginEditor(*this);
}

// ── State serialisation ────────────────────────────────────────────────────────

void RC20PluginProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void RC20PluginProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

// ── Parameter layout ───────────────────────────────────────────────────────────

juce::AudioProcessorValueTreeState::ParameterLayout
RC20PluginProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Helper lambdas to reduce boilerplate.
    auto addFloat  = [&](const char* id, const char* name,
                         float min, float max, float step, float def)
    {
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { id, 1 }, name,
            juce::NormalisableRange<float>(min, max, step), def));
    };

    auto addBool   = [&](const char* id, const char* name, bool def)
    {
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID { id, 1 }, name, def));
    };

    auto addChoice = [&](const char* id, const char* name,
                         juce::StringArray choices, int def)
    {
        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID { id, 1 }, name, choices, def));
    };

    // ── Global ────────────────────────────────────────────────────────────────
    addFloat(ParameterIDs::drift,        "Drift",        0.0f,  1.0f, 0.001f, 0.0f);
    addFloat(ParameterIDs::output_level, "Output Level", -24.0f, 6.0f, 0.1f,  0.0f);

    // ── Noise ─────────────────────────────────────────────────────────────────
    addBool  (ParameterIDs::noise_bypass, "Noise Bypass", false);
    addFloat (ParameterIDs::noise_amount, "Noise Amount", 0.0f, 1.0f, 0.001f, 0.0f);
    addChoice(ParameterIDs::noise_type,   "Noise Type",
              { "Tape Hiss", "Vinyl Crackle", "Electrical Hum" }, 0);

    // ── Wobble ────────────────────────────────────────────────────────────────
    addBool  (ParameterIDs::wobble_bypass, "Wobble Bypass", false);
    addFloat (ParameterIDs::wobble_amount, "Wobble Amount", 0.0f, 1.0f, 0.001f, 0.0f);
    addChoice(ParameterIDs::wobble_mode,   "Wobble Mode",
              { "Slow", "Medium", "Fast" }, 0);

    // ── Distortion ────────────────────────────────────────────────────────────
    addBool  (ParameterIDs::distortion_bypass, "Distortion Bypass", false);
    addFloat (ParameterIDs::distortion_amount, "Distortion Amount", 0.0f, 1.0f, 0.001f, 0.0f);
    addChoice(ParameterIDs::distortion_mode,   "Distortion Mode",
              { "Tape", "Tube", "Transistor" }, 0);

    // ── Space ─────────────────────────────────────────────────────────────────
    addBool  (ParameterIDs::space_bypass, "Space Bypass", false);
    addFloat (ParameterIDs::space_amount, "Space Amount", 0.0f, 1.0f, 0.001f, 0.0f);
    addChoice(ParameterIDs::space_mode,   "Space Mode",
              { "Room", "Hall", "Plate" }, 0);
    addFloat (ParameterIDs::space_size,   "Space Size",  0.0f, 1.0f, 0.001f, 0.5f);
    addFloat (ParameterIDs::space_tone,   "Space Tone",  0.0f, 1.0f, 0.001f, 0.5f);

    // ── Magic ─────────────────────────────────────────────────────────────────
    addBool  (ParameterIDs::magic_bypass, "Magic Bypass", false);
    addFloat (ParameterIDs::magic_amount, "Magic Amount", 0.0f, 1.0f, 0.001f, 0.0f);
    addChoice(ParameterIDs::magic_mode,   "Magic Mode",
              { "Bit Crush", "Decimate", "Combined" }, 0);

    // ── Limit ─────────────────────────────────────────────────────────────────
    addBool  (ParameterIDs::limit_bypass, "Limit Bypass", false);
    addFloat (ParameterIDs::limit_amount, "Limit Amount", 0.0f, 1.0f, 0.001f, 0.0f);
    addChoice(ParameterIDs::limit_mode,   "Limit Mode",
              { "Soft", "Medium", "Hard" }, 0);

    return { params.begin(), params.end() };
}

// ── Plugin entry point (required by JUCE) ─────────────────────────────────────

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RC20PluginProcessor();
}
