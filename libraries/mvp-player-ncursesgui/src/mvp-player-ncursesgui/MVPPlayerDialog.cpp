#include "MVPPlayerDialog.hpp"
#include "fileUtils.hpp"

#include <boost/thread.hpp>

#include <iostream>

namespace mvpplayer
{
namespace gui
{
namespace ncurses
{
namespace
{
    int kbhit()
    {
        struct timeval tv;
        fd_set fds;
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
        select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
        return FD_ISSET(STDIN_FILENO, &fds);
    }
}

MVPPlayerDialog::MVPPlayerDialog( CDKSCREEN *cdkScreen, const int width, const int height, const int x, const int y )
: _cdkScreen( cdkScreen )
, _childwin( nullptr )
, _playlist( nullptr )
, _isPlaying( false )
{
    initWin( "Hit play to select a track or a playlist..." );
}

MVPPlayerDialog::~MVPPlayerDialog()
{
    boost::mutex::scoped_lock lock( _mutexGui );
    destroyCDKDialog( _childwin );
    if ( _playlist )
    {
        destroyCDKItemlist( _playlist );
    }
}

void MVPPlayerDialog::initWin( const std::string & currentTrack, const bool playButton )
{
    boost::mutex::scoped_lock lock( _mutexGui );
    if ( _childwin )
    {
        destroyCDKDialog( _childwin );
    }
    init_pair( kColorMessage, COLOR_WHITE, COLOR_BLUE );
    char* msg[1];
    if ( currentTrack.length() == 0 )
    {
        msg[0] = "Music player";
    }
    else
    {
        msg[0] = const_cast<char*>( currentTrack.c_str() );
    }
    char* buttons[10];
    buttons[0] = "<<   ";
    if ( playButton )
    {
        buttons[1] = "</48>|> ";
    }
    else
    {
        buttons[1] = "</49>|| ";
    }
    buttons[2] = "   >>";
    buttons[3] = "\nExit";
    _childwin = newCDKDialog( _cdkScreen, CENTER, CENTER, msg, 1, buttons, 4, COLOR_PAIR(kColorMessage) | A_REVERSE | A_BOLD, true, true, false );
    setCDKDialogBackgroundColor( _childwin, "</5>" );
    assert( _childwin != NULL );
    refreshCDKScreen( _cdkScreen );
}

void MVPPlayerDialog::initPlaylist( const std::vector<std::string> & playlistItemsStr )
{
    std::vector<char*> itemsChars( playlistItemsStr.size() );
    std::vector<std::unique_ptr<char[]> > itemsAlloc( playlistItemsStr.size() );
    std::size_t i = 0;
    for( const std::string & itemStr: playlistItemsStr )
    {
        itemsAlloc[i].reset( new char[ itemStr.size() + 1 ] );
        strcpy( itemsAlloc[i].get(), itemStr.c_str() );
        itemsChars[i] = itemsAlloc[i].get();
        ++i;
    }

    boost::mutex::scoped_lock lock( _mutexGui );
    if ( _playlist )
    { destroyCDKItemlist( _playlist ); }

    _playlist = newCDKScroll( _cdkScreen, CENTER, TOP, RIGHT, 10, 78, "<C>Playlist", &itemsChars[0], itemsChars.size(), TRUE, A_REVERSE | A_BOLD, TRUE, FALSE );
    setCDKScrollBackgroundColor( _playlist, "</5>" );
    refreshCDKScreen( _cdkScreen );
}

void MVPPlayerDialog::openedPlaylist( const std::vector<m3uParser::PlaylistItem> & playlistItems )
{
    typedef std::vector<m3uParser::PlaylistItem> VecM3uItems;
    _playlistItemsStr.clear();
    std::for_each( playlistItems.begin(), playlistItems.end(),
        [this](typename VecM3uItems::const_reference item){ _playlistItemsStr.push_back( item.infos.size() ? item.infos : item.filename.string() ); }
    );

    initPlaylist( _playlistItemsStr );
}

void MVPPlayerDialog::setCurrentTrack( const boost::filesystem::path & filename )
{
    initWin( filename.string(), !_isPlaying );
}

void MVPPlayerDialog::setIconStop()
{
    _isPlaying = true;
    initWin( "", !_isPlaying );
}

void MVPPlayerDialog::setIconPlay()
{
    _isPlaying = false;
    initWin( "Hit play to resume track", !_isPlaying );
}

void MVPPlayerDialog::setPlaylistItemIndex( const int row )
{
    if ( _playlist )
    {
        boost::mutex::scoped_lock lock( _mutexGui );
        setCDKScrollCurrentItem( _playlist, row );
    }
}

int MVPPlayerDialog::exec()
{
    // Loop until user hits 'q' to quit
    {
        boost::mutex::scoped_lock lock( _mutexGui );
        refreshCDKScreen( _cdkScreen );
    }
    while( _childwin->exitType != vESCAPE_HIT )
    {
        int selection;

        while( !kbhit() )
        { boost::this_thread::sleep( boost::posix_time::milliseconds( 20 ) ); }

        {
            boost::mutex::scoped_lock lock( _mutexGui );
            const chtype input = static_cast<chtype>( getcCDKObject (ObjOf (_childwin) ) );
            drawCDKDialog( _childwin, ObjOf (_childwin)->box );
            wrefresh( _childwin->win );
            selection = injectCDKDialog( _childwin, input );
        }

        switch( selection )
        {
            case 0:
            {
                signalViewHitPrevious();
                break;
            }
            case 1:
            {
                if ( _isPlaying )
                {
                    signalViewHitStop();
                }
                else
                {
                    boost::filesystem::path item;
                    {
                        boost::mutex::scoped_lock lock( _mutexGui );
                        item = openFile( _cdkScreen, "Open file or playlist", "*" );
                    }

                    signalSequencial(
                        [this, &item]()
                        {
                            signalViewClearPlaylist();
                            signalViewHitPlay( item.string() );
                        }
                    );
                }
                break;
            }
            case 2:
            {
                signalViewHitNext();
                break;
            }
            case 3:
            {
                _childwin->exitType = vESCAPE_HIT;
                break;
            }
        }

        {
            boost::mutex::scoped_lock lock( _mutexGui );
            refreshCDKScreen( _cdkScreen );
        }
    }
    
    return 0;
}

void MVPPlayerDialog::clearPlaylist()
{
    boost::mutex::scoped_lock lock( _mutexGui );
    if ( _playlist )
    { destroyCDKItemlist( _playlist ); }
    refreshCDKScreen( _cdkScreen );
}

void MVPPlayerDialog::addTrack( const boost::filesystem::path & filename )
{
    _playlistItemsStr.push_back( filename.string() );
    initPlaylist( _playlistItemsStr );
}

}
}
}
