#include "IPeer.hpp"
#include <boost/exception/all.hpp>
#include <boost/lexical_cast.hpp>

namespace mvpplayer
{
namespace network
{

IPeer::IPeer( boost::asio::io_service *ioService )
: _ioScopedService( ioService ? nullptr : new boost::asio::io_service() )
, _ioService( ioService ? ioService : _ioScopedService.get() )
, _commandLen( 0 )
, _stop( false )
, _socket( *_ioService )
{
    if ( _ioScopedService )
    {
        _ioThread.reset( new boost::thread( boost::bind( &boost::asio::io_service::run, _ioService ) ) );
    }
}

IPeer::~IPeer()
{
    try
    {
        if ( _ioThread )
        {
            _ioThread->interrupt();
            _ioThread->join();
        }
        disconnect();
    }
    catch( ... )
    {
        std::cerr << ::boost::current_exception_diagnostic_information() << std::endl;
    }
}

void IPeer::start()
{
    _stop = false;
    _receivingThread.reset( new boost::thread( boost::bind( &This::handleReceiving, this ) ) );
}

std::string IPeer::address() const
{
    return _socket.remote_endpoint().address().to_string() + ":" + 
           boost::lexical_cast<std::string>( _socket.remote_endpoint().port() );
}

void IPeer::presentation( const PeerInfo & peerInfo )
{
    // Serialize peerInfo
    boost::asio::streambuf buf;
    std::ostream os( &buf );
    boost::archive::text_oarchive ar( os );
    ar << peerInfo;

    send( Data( boost::asio::buffer_cast<const boost::uint8_t*>( buf.data() ), buf.size() ) );
}

void IPeer::handleReceiving()
{
    try
    {
        while( !_stop )
        {
            boost::this_thread::interruption_point();
            // Read event
            IEvent *event = nullptr;
            recv( event );

            if ( event )
            {
                try
                {
                    signalEvent( *event );
                }
                catch( ... )
                {
                    std::cerr << ::boost::current_exception_diagnostic_information() << std::endl;
                }
            }
        }
    }
    catch( boost::thread_interrupted& )
    {}
    catch( ... )
    {
        std::cerr << ::boost::current_exception_diagnostic_information() << std::endl;
    }
}

void IPeer::disconnect()
{
    _stop = true;

    if ( _socket.is_open() )
    {
        try
        {
            _socket.cancel();
            _socket.shutdown( boost::asio::ip::tcp::socket::shutdown_both );
            _socket.close();
        }
        // Ignore errors
        catch( ... )
        {}
    }

    if ( _receivingThread )
    {
        _receivingThread->interrupt();
        if ( _ioScopedService )
        {
            _ioScopedService->stop();
        }
        _receivingThread->join();
        _receivingThread.reset( nullptr );
    }


    if ( _ioScopedService )
    {
        _ioScopedService->stop();
    }
}

boost::system::error_code IPeer::readEvent( IEvent *&event, const boost::uint32_t cmdLen )
{
    boost::system::error_code error;
    // read serialized data
    boost::asio::streambuf buf;
    boost::asio::read( _socket, buf.prepare( cmdLen ), error );
    buf.commit( cmdLen );

    // Unserialize
    std::istream is( &buf );
    IArchive archive( is );
    logic::registerClassInArchive( archive );

    archive >> event;
    return error;
}

boost::system::error_code IPeer::sendEvent( const IEvent * event )
{
    // Serialize event
    boost::asio::streambuf buf;
    std::ostream os( &buf );
    OArchive archive( os );
    logic::registerClassInArchive( archive );

    archive << event;

    return send( Data( boost::asio::buffer_cast<const boost::uint8_t*>( buf.data() ), buf.size() ) );
}

boost::system::error_code IPeer::send(  const Data & data )
{
    boost::system::error_code error;

    // Prepare buffers
    if ( !data.length )
    { return boost::system::errc::make_error_code( boost::system::errc::message_size ); }

    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back( boost::asio::buffer( &data.length, sizeof( data.length ) ) );
    buffers.push_back( boost::asio::buffer( &data.buffer[0], data.length ) );

    boost::asio::write( _socket, buffers, boost::asio::transfer_all(), error );

    // If there was an error, shutdown the connection
    if ( error || !_socket.is_open() )
    { _stop = true; }

    return error;
}

void IPeer::receivePeerInfo()
{
    recv( _peerInfo );
}

}
}
