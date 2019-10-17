#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <string>
#include <map>
#include <iterator>
#include <fstream>
#include <vector>
//#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define request_buffer_size 1024
class Server
{
private:
	class Client;
public:
	int server_listen(int port);
	void server_stop();
	~Server();
	void set_static(std::string path);
	void disable_static();
	bool is_static();
	std::string get_static();
	void handle_static(std::map<std::string, std::string> &req, Client &client);
	virtual void handle_custom_event(std::map<std::string, std::string> &req, Client &client);
	std::string server_name = "myserver";
protected:
	bool listening = false;
	void handle_connection(SOCKET &client, SOCKADDR_IN &client_addr, int addr_size);
	std::map<std::string, std::string> parse_request(char *request);
	bool static_get = false;
	std::string static_path;
	class Client
	{
	public:
		bool sent = false;
		Client(SOCKET &client,std::string &server_name);
		void send_text_response(std::string &resp);
		void send_file_response(char *filepath);
	private:
		std::vector<char> get_file(char *filepath);
		void send_response(std::vector<char> &resp, char *resp_code = (char*)"200 ok", char *cont_type = (char*)"text/html;charset=utf-8");
		void send_raw(char *buff,int len);
		SOCKET client;
		std::string server_name;
		
	};
	
};