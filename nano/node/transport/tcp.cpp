#include <nano/node/node.hpp>
#include <nano/node/transport/tcp.hpp>

nano::message_sink_tcp::message_sink_tcp (nano::node & node_a, std::shared_ptr<nano::socket> socket_a) :
node (node_a),
socket (socket_a)
{
}

size_t nano::message_sink_tcp::hash_code () const
{
	std::hash<::nano::tcp_endpoint> hash;
	return hash (socket->remote_endpoint ());
}

bool nano::message_sink_tcp::operator== (nano::message_sink const & other_a) const
{
	bool result (false);
	auto other_l (dynamic_cast<nano::message_sink_tcp const *> (&other_a));
	if (other_l != nullptr)
	{
		return *this == *other_l;
	}
	return result;
}

void nano::message_sink_tcp::send_buffer_raw (boost::asio::const_buffer buffer_a, std::function<void(boost::system::error_code const &, size_t)> const & callback_a) const
{
	socket->async_write (buffer_a, callback_a);
}

std::function<void(boost::system::error_code const &, size_t)> nano::message_sink_tcp::callback (std::shared_ptr<std::vector<uint8_t>> buffer_a, nano::stat::detail detail_a, std::function<void(boost::system::error_code const &, size_t)> const & callback_a) const
{
	return [ buffer_a, node = std::weak_ptr<nano::node> (node.shared ()), detail_a, callback_a ](boost::system::error_code const & ec, size_t size_a)
	{
		if (auto node_l = node.lock ())
		{
			if (ec == boost::system::errc::host_unreachable)
			{
				node_l->stats.inc (nano::stat::type::error, nano::stat::detail::unreachable_host, nano::stat::dir::out);
			}
			if (!ec)
			{
				node_l->stats.add (nano::stat::type::traffic, nano::stat::dir::out, size_a);
				node_l->stats.inc (nano::stat::type::message, detail_a, nano::stat::dir::out);
				if (callback_a)
				{
					callback_a (ec, size_a);
				}
			}
		}
	};
}

std::string nano::message_sink_tcp::to_string () const
{
	return boost::str (boost::format ("TCP: %1%") % socket->remote_endpoint ());
}