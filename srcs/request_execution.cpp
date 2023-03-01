#include "../includes/webserv.hpp"

int check_allowed_methods_request(std::string method, std::vector<std::string> allowed_methods)
{
    for (std::vector<std::string>::const_iterator it = allowed_methods.begin(); it != allowed_methods.end(); it++)
    {
        if (*it == method)
            return 0;
    }
    return 1;
}

void handle_request_execution(Request_parser &req_pars, Location &location_chosen, Static_response &rsp_hdl, int fd)
{
    rsp_hdl.setFd(fd);
    if (req_pars.getIsShortBody())
    {
        if (req_pars.getBytesInsideBody() > rsp_hdl.get_client_body_size())
        {
            rsp_hdl.generic_server_response(413);
            return ;
        }
    }
    if (req_pars.getIsLongBody())
    {
        if (req_pars.getBytesInsideFileBody() > rsp_hdl.get_client_body_size())
        {
            rsp_hdl.generic_server_response(413);
            return ;
        }
    }
    
    if (check_allowed_methods_request(req_pars.getRequestMethod(), location_chosen.getAllowedMethods()))
    {
        rsp_hdl.generic_server_response(405);
        return;
    }
    req_pars.setRootDirectory(location_chosen.getRootDirectory());
    req_pars.setIndex(location_chosen.getIndex());
    req_pars.setTitle(location_chosen.get_title());
    rsp_hdl.setAllowedMethod(true);
    if (location_chosen.getCgiPass().empty() && location_chosen.get_upload_store().empty())
    {

        if (req_pars.getRequestMethod() == "POST")
        {
            rsp_hdl.generic_server_response(405);
            return;
        }
        if (req_pars.getRequestMethod() == "DELETE")
        {
            rsp_hdl.setRequestParser(req_pars);
            rsp_hdl.delete_resource();
            return ;
        }
        if (req_pars.getRequestMethod() == "GET")
        {
            rsp_hdl.setRequestParser(req_pars);
            rsp_hdl.retrieve_static_resource();
            return ;
        }
        rsp_hdl.generic_server_response(400);
    }

    if (!location_chosen.getCgiPass().empty() && location_chosen.get_upload_store().empty())
    {
        
        req_pars.parse_request_cgi(req_pars.get_extension());
        rsp_hdl.set_binary_file(location_chosen.get_binary_file());
        Cgi_handler cgi_hdl(req_pars, rsp_hdl);
        try
        {
            cgi_hdl.execute_request();
        }
        catch (Cgi_handler::CgiScriptNotFound& e)
        {
            rsp_hdl.generic_server_response(404);
            return ;
        }
        catch (Cgi_handler::CgiScriptFailed& e)
        {
            rsp_hdl.generic_server_response(500);
            return ;
        }
        catch (Cgi_handler::CgiScriptInvalid& e)
        {
            rsp_hdl.generic_server_response(500);
            return ;
        }
        catch (Cgi_handler::CgiScriptIsDirectory& e)
        {
            rsp_hdl.generic_server_response(403);
            return ;
        }
        rsp_hdl.prepare_cgi_response(cgi_hdl.getResponseBuffersPtr());
    }
    if (!location_chosen.get_upload_store().empty())
    {
        rsp_hdl.set_upload_store(location_chosen.get_upload_store());
        try
        {
            rsp_hdl.upload_file();
        }
        catch(const Static_response::UploadFileBadRequest& e)
        {
            rsp_hdl.generic_server_response(400);
            return ;
        }
        catch(const Static_response::UploadFileFailed& e)
        {
            rsp_hdl.generic_server_response(500);
            return ;
        }        
       
        rsp_hdl.generic_server_response(201);
    }
}
