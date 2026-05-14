#pragma once

namespace CONFIG {
	constexpr float MIN_IN_DB = -24; // Min possible input gain (dB)
	constexpr float MAX_IN_DB = 24;// Max possible input gain (dB)
	constexpr float MAX_GAIN_RANGE_DB = 48; // Min + Max possible input gain range (dB)

	constexpr float PRE_NOTCH_HZ = 320;
	constexpr float PRE_NOTCH_Q = 5.4;
	constexpr float PRE_LP_HZ = 14000;
	constexpr float POST_NOTCH_HZ = 670;
	constexpr float POST_NOTCH_Q = 1;

	constexpr float POST_LP1_HZ = 6200;
	constexpr float POST_LP1_Q = 0.707;
	constexpr float POST_LP2_HZ = 6500;
	constexpr float POST_LP2_Q = 0.707;

	constexpr float BASSCUT_NORMAL_HZ = 1000;

	constexpr float LO_NORMAL_HZ = 200;
	constexpr float LO_CUT_HZ = 700;
	constexpr float LO_BOOST_HZ = 50;

	constexpr float HI_CUT_HZ = 3000;
	constexpr float HI_BOOST_HZ = 1500;




}