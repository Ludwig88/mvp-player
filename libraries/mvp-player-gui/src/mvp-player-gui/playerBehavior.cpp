#include "playerBehavior.hpp"

namespace mvpplayer
{
namespace gui
{

/**
 * @brief Setup music player's behavior (connect signals between the model/view/presenter)
 * Connections should be done anywhere else than here.
 */
void setupMainBehavior( mvpplayer::MVPPlayerEngine & m, mvpplayer::gui::IMVPPlayerDialog & v, mvpplayer::logic::MVPPlayerPresenter & p )
{
    connectViewPresenter( v, p );
    connectPresenterModel( p, m );
}

/**
 * @brief Bind view to presenter
 */
void connectViewPresenter( mvpplayer::gui::IMVPPlayerDialog & v, mvpplayer::logic::MVPPlayerPresenter & p )
{
    //@{ Connections (behavior), note that the order is important
    // When we hit a button, we want to react by sending a the right event to the state machine
    v.signalViewHitButton.connect( boost::bind( &mvpplayer::logic::MVPPlayerPresenter::processCommand, &p, _1 ) );
    // When we hit clear playlist button, we want to react by sending a clear playlist event to the state machine
    v.signalViewClearPlaylist.connect( boost::bind( &mvpplayer::logic::MVPPlayerPresenter::processClearPlaylist, &p ) );
    p.signalModelClearedPlaylist.connect( boost::bind( &mvpplayer::gui::IMVPPlayerDialog::clearedPlaylist, &v ) );
    // When we hit start playlist button, we want to react by sending a start playlist event to the state machine
    v.signalViewStartPlaylist.connect( boost::bind( &mvpplayer::logic::MVPPlayerPresenter::processStartPlaylist, &p ) );
    // When we add a track on the view, we want to notify the presenter
    v.signalViewAddTrack.connect( boost::bind( &mvpplayer::logic::MVPPlayerPresenter::processAddTrack, &p, _1 ) );
    // When the presenter signalize that the model added a track, we want to react by adding the track in the view's playlist
    p.signalAddedTrack.connect( boost::bind( &mvpplayer::gui::IMVPPlayerDialog::addTrack, &v, _1 ) );
    // When we add playlist on the view, we want to notify the presenter
    v.signalViewAppendPlaylistTracks.connect( boost::bind( &mvpplayer::logic::MVPPlayerPresenter::processAppendPlaylistTrack, &p, _1 ) );
    // When we add a track items on the view, we want to notify the presenter
    v.signalViewAppendTrackItems.connect( boost::bind( &mvpplayer::logic::MVPPlayerPresenter::processAppendTrackItems, &p, _1 ) );
    // When the view want to play a playlist item, react by sending the event to the state machine
    v.signalViewHitPlaylistItem.connect( boost::bind( &mvpplayer::logic::MVPPlayerPresenter::processPlayItemAtIndex, &p, _1 ) );
    // When the view want to change track position, react by sending the event to the state machine
    v.signalViewHitTrackPosition.connect( boost::bind( &mvpplayer::logic::MVPPlayerPresenter::processSetTrackPosition, &p, _1 ) );
    // When the view want to change volume, react by sending the event to the state machine
    v.signalViewSetVolume.connect( boost::bind( &mvpplayer::logic::MVPPlayerPresenter::processSetVolume, &p, _1 ) );
    // Connect stop event to change play button to [*]
    p.signalPlayedTrack.connect( boost::bind( &mvpplayer::gui::IMVPPlayerDialog::setIconStop, &v ) );
    // Connect played event to display track filename function
    p.signalPlayedTrack.connect( boost::bind( &mvpplayer::gui::IMVPPlayerDialog::setCurrentTrack, &v, _1 ) );
    // Connect stop event to display empty track filename
    p.signalStopTrack.connect( boost::bind( &mvpplayer::gui::IMVPPlayerDialog::setCurrentTrack, &v, boost::filesystem::path() ) );
    // Connect stop event to change play button to |>
    p.signalStopTrack.connect( boost::bind( &mvpplayer::gui::IMVPPlayerDialog::setIconPlay, &v ) );
    // Connect 'Set button checked' event to the view
    p.signalCommandActive.connect( boost::bind( &mvpplayer::gui::IMVPPlayerDialog::setButtonChecked, &v, _1, _2 ) );
    // When the model plays another playlist track, inform the view
    p.signalPlayingItemIndex.connect( boost::bind( &mvpplayer::gui::IMVPPlayerDialog::setPlaylistItemIndex, &v, _2 ) );
    // When the presenter notify that it opened a playlist, inform the view
    p.signalOpenedPlaylist.connect( boost::bind( &mvpplayer::gui::IMVPPlayerDialog::openedPlaylist, &v, _1 ) );
    // When the presenter notify that the track length changed, inform the view
    p.signalTrackLengthChanged.connect( boost::bind( &mvpplayer::gui::IMVPPlayerDialog::setTrackLength, &v, _1 ) );
    // When the presenter notify that the track position changed, inform the view
    p.signalTrackPositionChanged.connect( boost::bind( &mvpplayer::gui::IMVPPlayerDialog::setTrackPosition, &v, _1, _2 ) );
    // When the presenter notify that the track position changed, inform the view
    p.signalSetVolume.connect( boost::bind( &mvpplayer::gui::IMVPPlayerDialog::setVolume, &v, _1 ) );
    //@}
}

/**
 * @brief Bind presenter to model
 */
void connectPresenterModel( mvpplayer::logic::MVPPlayerPresenter & p, mvpplayer::MVPPlayerEngine & m )
{
    // Connect clear playlist event to clear the engine's playlist
    p.signalClearPlaylist.connect( boost::bind( &mvpplayer::MVPPlayerEngine::clearPlaylist, &m ) );
    // Subscribe to clear playlist notifications to clear the view's playlist
    // Connect start playlist event to start the engine's playlist
    p.signalStartPlaylist.connect( boost::bind( &mvpplayer::MVPPlayerEngine::playList, &m ) );
    // When the presenter signalize that user added a track, we want to react by adding the track in the model's playlist
    p.signalAddTrack.connect( boost::bind( &mvpplayer::MVPPlayerEngine::addTrack, &m, _1 ) );
    // When the presenter signalize that user opened a playlist, inform the presenter
    p.signalAppendPlaylistTracks.connect( boost::bind( &mvpplayer::MVPPlayerEngine::openPlaylist, &m, _1 ) );
    // When the presenter signalize that user opened a playlist, inform the presenter
    p.signalAppendTrackItems.connect( boost::bind( &mvpplayer::MVPPlayerEngine::addTrackItems, &m, _1 ) );
    // When the presenter notify that we want to play an item of the playlist, inform the model
    p.signalPlayItemAtIndex.connect( boost::bind( &mvpplayer::MVPPlayerEngine::playPlaylistItem, &m, _1 ) );
    // Connect played event to the engine play file function
    p.signalPlayTrack.connect( boost::bind( &mvpplayer::MVPPlayerEngine::playFile, &m, _1 ) );
    // Connect stop event to stop the current played track
    p.signalStopTrack.connect( boost::bind( &mvpplayer::MVPPlayerEngine::stop, &m ) );
    // When the logic asks next track, we want to play next track
    p.signalNextTrack.connect( boost::bind( &mvpplayer::MVPPlayerEngine::playNext, &m ) );
    // When the logic asks previous track, we want to play previous track
    p.signalPreviousTrack.connect( boost::bind( &mvpplayer::MVPPlayerEngine::playPrevious, &m ) );
    // When the logic asks for 'restart track' play restart track
    p.signalRestartTrack.connect( boost::bind( &mvpplayer::MVPPlayerEngine::restart, &m ) );
    // When the logic asks for 'restart track' play restart track
    p.signalSetTrackPosition.connect( boost::bind( &mvpplayer::MVPPlayerEngine::setTrackPosition, &m, _1, mvpplayer::eSeekPositionPercent ) );
    // When the logic asks to change the volume, set the sound engine's volume
    p.signalSetVolume.connect( boost::bind( &mvpplayer::MVPPlayerEngine::setVolume, &m, _1 ) );
    // Process cleared playlist event when the model clear its playlist
    m.signalClearedPlaylist.connect( boost::bind( &mvpplayer::logic::MVPPlayerPresenter::processModelClearedPlaylist, &p ) );
    // When we add a track on the model, we want to notify the presenter
    m.signalTrackAddedToPlaylist.connect( boost::bind( &mvpplayer::logic::MVPPlayerPresenter::processAddedTrack, &p, _1 ) );
    // subscribe to model 'played track' signal
    m.signalPlayedTrack.connect( boost::bind( &mvpplayer::logic::MVPPlayerPresenter::processPlayed, &p, _1 ) );
    // When the model plays another playlist track, inform the view
    m.signalPlayingItemIndex.connect( boost::bind( &mvpplayer::logic::MVPPlayerPresenter::processPlayingItemIndex, &p, _1, _2 ) );
    // When the model updates track's length, inform the view
    m.signalTrackLength.connect( boost::bind( &mvpplayer::logic::MVPPlayerPresenter::processTrackLengthChanged, &p, _1 ) );
    // When the model updates track's position, inform the view
    m.signalPositionChanged.connect( boost::bind( &mvpplayer::logic::MVPPlayerPresenter::processTrackPositionChanged, &p, _1, _2 ) );
    // Subscribe to end of track notifications
    m.signalEndOfTrack.connect( boost::bind( &mvpplayer::logic::MVPPlayerPresenter::processEndOfTrack, &p ) );
    // When the model notify that it opened a playlist, inform the presenter
    m.signalOpenedPlaylist.connect( boost::bind( &mvpplayer::logic::MVPPlayerPresenter::processOpenedPlaylist, &p, _1 ) );
}

}
}
