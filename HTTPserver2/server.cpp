#include "server.h"

int Server::server_listen(int port)
{
	listening = true;
	int error_code;
	WSADATA WSAData;
	SOCKET server, client;
	SOCKADDR_IN server_addr, client_addr;
	error_code=WSAStartup(MAKEWORD(2, 0), &WSAData);
	if ( error_code !=0 )
	{
		std::cout << "error at WSAStartup " << error_code << std::endl;
		return error_code;
	}
	server = socket(AF_INET, SOCK_STREAM, 0);
	
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);

	error_code=bind(server, (SOCKADDR *)&server_addr, sizeof(server_addr));
	if (error_code != 0)
	{
		std::cout << "error at binding socket " << error_code << std::endl;
		return error_code;
	}
	
	error_code = listen(server, 0);
	if (error_code != 0)
	{
		std::cout << "error at socket listen " << error_code << std::endl;
		return error_code;
	}
	std::cout << "Listening for incoming connections at port "<< port << std::endl;

	int clientAddrSize = sizeof(client_addr);
	while (listening) 
	{
		if ((client = accept(server, (SOCKADDR *)&client_addr, &clientAddrSize)) != INVALID_SOCKET)
		{
			handle_connection(client,client_addr,clientAddrSize);
		}
	}
}
void Server::handle_connection(SOCKET &client,SOCKADDR_IN &client_addr,int addr_size)
{
	char buffer[request_buffer_size];
	int error_code;
	std::map<std::string, std::string> parsedmap;
	std::cout << "Client: " << getpeername(client, (SOCKADDR *)&client_addr, &addr_size) << std::endl;
	recv(client, buffer, sizeof(buffer), 0);
	parsedmap=parse_request(buffer);
	if (parsedmap[std::string("error")] == std::string("false"))
	{

		Client client1(client,this->server_name);
		if (static_get)
		{
			handle_static(parsedmap,client1);
		}
		handle_custom_event(parsedmap, client1);

	}
	memset(buffer, 0, sizeof(buffer));
	closesocket(client);
	
}
std::map<std::string, std::string> Server::parse_request(char *request)
{
	std::string tmp,reqstr = request;
	std::string::size_type l_index = 0, n_index = 0,s_index=0;
	std::map<std::string,std::string> reqmap;
	reqmap.insert(std::pair<std::string, std::string>(std::string("error"), std::string("true")));
	if (reqstr.find("HTTP") == std::string::npos)
	{
		std::cout << "not an http request:\n"+reqstr << std::endl;
		
		return reqmap;
	}
	
	//reqstr = reqstr.substr(0, reqstr.rfind('\n')) + std::string("\0");
	l_index = n_index;
	n_index = reqstr.find("\n", l_index + 1);
	tmp = reqstr.substr(l_index, n_index - l_index);
	s_index = tmp.find(' ');
	reqmap.insert(std::pair<std::string, std::string>(std::string("method"),tmp.substr(0, s_index)));
	reqmap.insert(std::pair<std::string, std::string>(std::string("route"), tmp.substr(s_index+1, tmp.find(' ', s_index + 1)-(s_index + 1))));
	while (n_index != std::string::npos) 
	{
		tmp = reqstr.substr(l_index, n_index - l_index);
		s_index = tmp.find(": ");
		if (s_index != std::string::npos) 
		{
			reqmap.insert(std::pair<std::string, std::string>(tmp.substr(0, s_index), tmp.substr(s_index+2, tmp.length()- s_index)));
		}
		l_index = n_index + 1;
		n_index = reqstr.find_first_of("\n", l_index + 1);
	}
	
	if (reqmap.at("method") == std::string("POST")) 
	{
		s_index = reqstr.rfind("Content-Length: ")+15;
		reqmap.insert(std::pair<std::string, std::string>(std::string("content"), reqstr.substr(s_index, reqstr.length() - s_index)));
	}
	reqmap.at(std::string("error")) = std::string("false");
	return reqmap;
	
}
void Server::server_stop()
{
	listening = false;
}

void Server::set_static(std::string path)
{
	static_get = true;
	
	
	static_path = path;
}
void Server::disable_static()
{
	static_get = false;
}
bool Server::is_static()
{
	return static_get;
}
std::string Server::get_static()
{
	return static_path;
}
void Server::handle_static(std::map<std::string, std::string> &req, Client &client)
{
	client.send_file_response((char*)std::string(static_path + req["route"]).c_str());
}
void Server::handle_custom_event(std::map<std::string, std::string> &req, Client &client)
{

}
Server::~Server()
{
	server_stop();
}


Server::Client::Client(SOCKET &client, std::string &server_name)
{
	this->client = client;
	this->server_name = server_name;
}
void Server::Client::send_raw(char *buff,int len)
{
	this->sent = true;
	send(this->client, buff, len, 0);
	
}
std::vector<char> Server::Client::get_file(char *fp)
{
	std::fstream file;
	std::vector<char> cont;
	file.open(fp, std::ios::in|std::ios::binary);
	
	if (file)
	{
		
		file.seekg(0, std::ios::end);
		std::ifstream::pos_type file_size = file.tellg();
		file.seekg(0, std::ios::beg);
		cont.resize((int)file_size);
		if (file.read(&cont[0], file_size) )
		{
			
		}
		else
		{
			cont.resize(1);
			cont[0] = 0;
			std::cout << "can't read from " << fp << std::endl;
		}
	}
	else
	{
		cont.resize(1);
		cont[0] = 0;
		std::cout << "error cant open file " << fp << " to read from." << std::endl;
		
	}
	file.close();
	return cont;
}
void Server::Client::send_response(std::vector<char> &resp,char *resp_code,char *cont_type)
{
	
	std::string response("HTTP/1.1 ");
	response += resp_code + std::string("\n");
	response += "Server: "+this->server_name+"\n";
	response += "Content-Type: " + std::string(cont_type) + "\n";
	response += "Content-Length: " +std::to_string(resp.size()) + "\n\n";
	char *raw = new char[response.length() + resp.size()];
	memcpy(raw, response.c_str(), response.length());
	memcpy(&raw[response.length()], &resp[0], resp.size());
	send_raw(raw, response.length() + resp.size());

}
void Server::Client::send_file_response(char *fp)
{
	std::vector<char> e404; 
	e404.resize(50);
	memcpy(&e404[0], "<h1>404</h1><hr/>\0", 18);
	std::vector<char> cont=get_file(fp);
	if (cont.size() == 1 && cont[0] == 0)
	{
		//send_response(e404,(char*)"404 failed");
	}
	else
	{
		send_response(cont);
	}
}
void Server::Client::send_text_response(std::string &resp)
{
	std::vector<char> to_send;
	to_send.resize(resp.length());
	memcpy(&to_send[0], resp.c_str(),resp.length());
	send_response(to_send);
}