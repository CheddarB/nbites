#pragma once

#include "RoboGrams.h"
#include "AlSpeech.h"
#include "alcommon/albroker.h"
#include "Audio.pb.h"

namespace man {
	namespace audio {

		class AudioEnactorModule : public portals::Module {

		public:
			AudioEnactorModule(boost::shared_ptr<AL::ALBroker> broker, portals::OutPortal<messages::Audio> out);
			virtual ~AudioEnactorModule() {}

			portals::InPortal<messages::Audio> AudioIn;

		protected:
			virtual void run_();
			AlSpeech alspeech;
		};

	}
}
