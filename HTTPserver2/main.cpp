
#include "server.h"
#include <iostream>
class Myserver : public Server
{//here you can extend the server to do diffrent things (from templates to handling a database)
	void handle_custom_event(std::map<std::string, std::string> &req, Client &client) override
	{
		if (req["method"] == std::string("GET")) 
		{
			std::string response;
			response = "<h2><i>you tried to get: </i><br><b> " + req["route"]+"</b></h2><hr/>";
			client.send_text_response(response);
		}
	}
};
void main()
{
	Myserver myserver;
	myserver.server_name = "my custom server.";
	myserver.set_static("./public");
	myserver.server_listen(8000);
	std::system("pause");
}