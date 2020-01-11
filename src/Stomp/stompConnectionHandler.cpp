//
// Created by omerlux@wincs.cs.bgu.ac.il on 10/01/2020.
//

#include "stompConnectionHandler.h"
#include <string>
#include <boost/lexical_cast.hpp>// for lexical_cast()

using boost::asio::ip::tcp;

using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::string;

stompConnectionHandler::stompConnectionHandler(string host, short port, User* user): host_(host), port_(port), io_service_(), socket_(io_service_), user(user), registered(false){}

stompConnectionHandler::~stompConnectionHandler() {
    close();
}

bool stompConnectionHandler::connect(std::string host, short port) {
    //------------------- start edit 11/1 ------------------------
    this->host_ =host;
    this->port_ =port;
    //------------------- end edit 11/1 --------------------------

    std::cout << "Starting connect to "
              << host_ << ":" << port_ << std::endl;
    try {
        tcp::endpoint endpoint(boost::asio::ip::address::from_string(host_), port_); // the server endpoint
        boost::system::error_code error;
        socket_.connect(endpoint, error);
        if (error)
            throw boost::system::system_error(error);
    }
    catch (std::exception& e) {
        std::cerr << "Connection failed (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}

bool stompConnectionHandler::getBytes(char bytes[], unsigned int bytesToRead) {
    size_t tmp = 0;
    boost::system::error_code error;
    try {
        while (!error && bytesToRead > tmp ) {
            tmp += socket_.read_some(boost::asio::buffer(bytes+tmp, bytesToRead-tmp), error);
        }
        if(error)
            throw boost::system::system_error(error);
    } catch (std::exception& e) {
        std::cerr << "receive failed (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}

bool stompConnectionHandler::sendBytes(const char bytes[], int bytesToWrite) {
    int tmp = 0;
    boost::system::error_code error;
    try {
        while (!error && bytesToWrite > tmp ) {
            tmp += socket_.write_some(boost::asio::buffer(bytes + tmp, bytesToWrite - tmp), error);
        }
        if(error)
            throw boost::system::system_error(error);
    } catch (std::exception& e) {
        std::cerr << "recv failed (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}

bool stompConnectionHandler::getFrame(std::string& line) {      //get bytes Frame (from server) -> translate to string Frame
    return getFrameAscii(line, '\0');
}

bool stompConnectionHandler::sendFrame(std::string& line) {     //get string Frame -> translate to bytes Frame (to server)
    return sendFrameAscii(line, '\0');
}


bool stompConnectionHandler::getFrameAscii(std::string& frame, char delimiter) {
    char ch;
    // Stop when we encounter the null character.
    // Notice that the null character is not appended to the frame string.
    try {
        do{
            if(!getBytes(&ch, 1))
            {
                return false;
            }
            if(ch!='\0')
                frame.append(1, ch);
        }while (delimiter != ch);
    } catch (std::exception& e) {
        std::cerr << "recv failed2 (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}


bool stompConnectionHandler::sendFrameAscii(const std::string& frame, char delimiter) {
    bool result=sendBytes(frame.c_str(),frame.length());
    if(!result) return false;
    return sendBytes(&delimiter,1);
}

// Close down the connection properly.
void stompConnectionHandler::close() {
    try{
        socket_.close();
    } catch (...) {
        std::cout << "closing failed: connection already closed" << std::endl;
    }
}

// stompSendProcess
void stompConnectionHandler::stompSendProcess(std::string &input) {
    //------------------- start edit 11/1 ------------------------
    // string first_word = input.substr(0,input.find(" "));
    std::vector <string> inputBySpace ;
    std::string delimiter = " ";

    //Parsing the string
    size_t pos = 0;
    std::string token;
    while ((pos = input.find(delimiter)) != std::string::npos) {
        token = input.substr(0, pos);
        inputBySpace.push_back(token);
        input.erase(0, pos + delimiter.length());
    }
    inputBySpace.push_back(input);
    std::string first_word = inputBySpace[0];

        ///------------------------Login send----------------------------------------------------
    if(first_word == "login"){
        size_t pos = input.find(":");
        std::string host = inputBySpace[1].substr(0,pos);
        std::string port_str = inputBySpace[1].substr(pos+1, inputBySpace[1].length()-1);
        short port = std::stoul (port_str,nullptr,0);

        if(!connect(host, port)){
            std::cerr<<"Cannot connect to " << host << ":" <<port << std::endl;
        }
        else{
            if(registered==false){
                registered =true;
                user->setName(inputBySpace[2]);                         // [2] is the user name
            }

            std::stringstream ss;
            ss << "CONNECT\n" <<
               "accept-version:1.2\n" <<
               "host:stomp.cs.bgu.ac.il\n" <<
               "login:"+inputBySpace[2]+"\n" <<
               "passcode:"+inputBySpace[3]+"\n\n^@";                            // [3] is the password
            std::string frame = ss.str();
            sendFrame(frame);
        }

        ///------------------------Join send----------------------------------------------------
    }else if(first_word == "join"){
        string topic = inputBySpace[1];                                     // [1] is the topic name
        int topicId = user->subTopic(topic);
        int receiptId = user->addReceiptId();

        std::stringstream ss2;
        ss2 << "SUBSCRIBE\n" <<
           "destination:"+topic+"\n" <<
           "id:"+std::to_string(topicId)+"\n" <<
           "receipt:"+std::to_string(receiptId)+"\n\n^@";
        std::string frame = ss2.str();
        sendFrame(frame);

        ///------------------------Add send----------------------------------------------------
    }else if(first_word == "add"){
        Book* new_book = new Book(inputBySpace[2],inputBySpace[1],nullptr); //creating new book with no loners
        user->addBook(new_book);

        std::stringstream ss;
        ss << "SEND\n" <<
            "destination:"+inputBySpace[1]+"\n" <<                              // [1] is the topic name
            "\n" <<
            user->getName()+" has added the book "+inputBySpace[2]+"\n\n^@";    // [2] is the book name
        std::string frame = ss.str();
        sendFrame(frame);

        ///------------------------Borrow send----------------------------------------------------
    }else if(first_word == "borrow"){
        std::stringstream ss;
        ss << "SEND\n" <<
            "destination:"+inputBySpace[1]+"\n" <<                              // [1] is the topic name
            "\n" <<
            user->getName()+" wish to borrow "+inputBySpace[2]+"\n\n^@";        // [2] is the book name
        std::string frame = ss.str();
        sendFrame(frame);

        ///------------------------Return send----------------------------------------------------
    }else if(first_word == "return"){
        string loner="";
        for(Book* book: user->getInventory()){                  // searching for the book we want to return
            if(book->getBookname() == inputBySpace[2]) {        // [2] is the book name
                loner = book->getLoner();                       // if loner is real name, than we will delete the book and send the frame
                if(loner != "") {
                    user->getInventory().erase(
                            std::remove(user->getInventory().begin(), user->getInventory().end(), book));
                    delete book;

                    std::stringstream ss;
                    ss << "SEND\n" <<
                       "destination:" + inputBySpace[1] + "\n" <<
                       "\n" <<
                       " returning " + inputBySpace[2] + " to " + loner + "\n\n^@";
                    std::string frame = ss.str();
                    sendFrame(frame);
                }
                break;
            }
        }

        ///------------------------Status send----------------------------------------------------
    }else if(first_word == "status"){
        std::stringstream ss;
        ss << "SEND\n" <<
           "destination:"+inputBySpace[1]+"\n" <<             // [1] is the topic name
           "\n" <<
           "book status\n\n^@";
        std::string frame = ss.str();
        sendFrame(frame);

        ///------------------------Logout send----------------------------------------------------
    }else if(first_word == "logout"){


        ///------------------------Exit send----------------------------------------------------
    }else if(first_word == "exit"){
    ///Logout and change flag to EXIT the program
    }

//------------------- end edit 11/1 --------------------------
}














