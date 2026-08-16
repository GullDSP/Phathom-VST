<h1><strong>Phathom</strong></h1>
<p>Phathom is a bass preamp plugin targeted towards modern fuzzy / growly bass guitar tones. Featuring dynamic transformer and CMOS-like saturation algorithms, and pre-distortion bass and treble filters to change how it reacts to picking dynamics and input timbre. </p>

<img width="745" height="533" alt="image" src="https://github.com/user-attachments/assets/7e43f065-81ca-4ba9-83ff-87a33be34399" />


<h3>Download</h3>
https://github.com/GullDSP/Phathom-VST/releases
<h3>Demo</h3>
<a href="https://www.youtube.com/watch?v=Skig5GgicCw">
  <img src="https://markdown-videos-api.jorgenkh.no/url?url=https%3A%2F%2Fwww.youtube.com%2Fwatch%3Fv%3DSkig5GgicCw" alt="Phathom Demo" title="Phathom Demo"/>
</a>

Demo bass guitar played by Rich Gray @annihilatorbass on [Fiverr](https://www.fiverr.com/annihilatorbass)
<h3>Features</h3>
<ul>
<li>Hysteretic stateful saturation. Instead of a static waveshaper, the distortions here react to a combination of current and past input levels and frequencies, similar to a tape, transformer or modern CMOS pedal. This gives the distortion an organic element, it reacts to playing style, dynamics and timbre.
</li>
<li>Choice of two styles of distortion</li>
<li>Pre-saturation low or high boost or cut to change the character of the saturation</li>
<li>Optional wet signal polarity inversion and dry signal equalisation </li>
<li>Automatic oversampling. Host sample rates under 80kHz are oversampled 4X and 80kHz to 160kHz 2X. The plugin is always zero latency regardless of oversampling status</li>
<li>Resizable UI, right click to select UI size</li>
</ul>
<h3>Parameters</h3>
<ul>
<li><strong>Input</strong> - Input level boost or cut, with overload indicator</li>
<li><strong>Drive</strong> - Sets the input level into the saturation module</li>
<li><strong>Style</strong> - Choose between Fuzzy and Clanky distortion models. Fuzzy is a smooth transformer like saturation, whereas Clanky is a CMOS style modern noisy distortion</li>
<li><strong>Mix</strong> - Dry / Wet mix</li>
<li><strong>Out</strong> - Output level boost or cut, with overload indicator</li>
<li><strong>P</strong> - Flips the polarity of the distorted signal, changing how it mixes back with the dry signal. Use this with caution, while it can create interesting scooped tones, in some cases it may cause some loss of bass frequencies. The nature of this bass and low mid loss may be desirable and so this should be judged by ear.</li>
<li><strong>Dry EQ</strong> - Apply the pre-saturation signal path to the dry input signal. This can be useful if feeding into other distortion effects.</li>


<h3>Acknowledgements</h3>
<ul>
<li>This project is built using the Steinberg VST 3 SDK(https://www.steinberg.net/developers/).</li>
<li>VST is a trademark of Steinberg Media Technologies GmbH.</li>
<img width="200" height="187" alt="XXXXXXXX_snapshot_2 0x" src="https://steinbergmedia.github.io/vst3_dev_portal/resources/licensing_3.png" />

</ul>
