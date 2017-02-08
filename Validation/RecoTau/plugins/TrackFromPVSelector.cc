////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "CommonTools/Utils/interface/StringCutObjectSelector.h"

#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"

#include <algorithm>
#include <memory>
#include <vector>


////////////////////////////////////////////////////////////////////////////////
// class definition
////////////////////////////////////////////////////////////////////////////////
class TrackFromPVSelector : public edm::EDProducer {
public:

  explicit TrackFromPVSelector(const edm::ParameterSet& iConfig);

  void produce(edm::Event& iEvent,const edm::EventSetup& iSetup) override;

private:

  double max_dxy_;
  double max_dz_;
  edm::EDGetTokenT<std::vector<reco::Vertex>> v_recoVertexToken_;
  edm::EDGetTokenT<std::vector<reco::Track>> v_recoTrackToken_;
};


////////////////////////////////////////////////////////////////////////////////
// construction/destruction
////////////////////////////////////////////////////////////////////////////////

//______________________________________________________________________________
TrackFromPVSelector::TrackFromPVSelector(const edm::ParameterSet& iConfig)
  : max_dxy_{iConfig.getParameter<double>("max_dxy")}
  , max_dz_{iConfig.getParameter<double>("max_dz")}
  , v_recoVertexToken_{consumes<std::vector<reco::Vertex>>(iConfig.getParameter<edm::InputTag>("srcVertex"))}
  , v_recoTrackToken_{consumes<std::vector<reco::Track>>(iConfig.getParameter<edm::InputTag>("srcTrack"))}
{
  produces<std::vector<reco::Track>>();
}

////////////////////////////////////////////////////////////////////////////////
// implementation of member functions
////////////////////////////////////////////////////////////////////////////////

//______________________________________________________________________________
void TrackFromPVSelector::produce(edm::Event& iEvent, edm::EventSetup const&)
{
  auto goodTracks = std::make_unique<std::vector<reco::Track>>();

  edm::Handle<std::vector<reco::Vertex>> vertices;
  edm::Handle<std::vector<reco::Track>> tracks;
  if (iEvent.getByToken(v_recoVertexToken_, vertices) && !vertices->empty() &&
      iEvent.getByToken(v_recoTrackToken_, tracks) && !tracks->empty())
    {
      auto const& vtxPos = vertices->front().position();
      std::copy_if(std::cbegin(*tracks), std::cend(*tracks), std::back_inserter(*goodTracks),
                   [this, &vtxPos](auto const& track) {
                     return fabs(track.dxy(vtxPos)) < max_dxy_ && fabs(track.dz(vtxPos)) < max_dz_;
                   });
    }
  iEvent.put(std::move(goodTracks));
}

DEFINE_FWK_MODULE(TrackFromPVSelector);
