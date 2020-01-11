//
// Created by omerlux@wincs.cs.bgu.ac.il on 10/01/2020.
//
#include <string>
#include <unordered_map>
#include "Book.h"
#include "User.h"
//------------------- start edit 11/1 ------------------------
User::User(): name(),inventory(), topic_idByClient_map(),receiptId_returned_map(), topicIdCounter(1), receiptIdCounter(1){};

User::~User(){
    for( Book *b:inventory){
        delete b;
    }
}

unordered_map<string, int> &User::get_topic_idByClient_map() { return topic_idByClient_map;}

unordered_map<int, bool> &User::get_receiptId_returned_map() { return receiptId_returned_map;}

vector<Book*> & User::getInventory() { return inventory;}

std::string User::getName() { return name;}

void User::setName(std::string name) { this->name=name;}

//adding a book to the inventory
void User::addBook(Book *book) {inventory.push_back(book);}
//------------------- end edit 11/1 --------------------------


///Topic functions
//subscribe to a sepcific topic - the client will use the #topicId instead
// returning the new topic_id, or the old one.
int User::subTopic(string &topic) {
    //------------------- start edit 11/1 ------------------------
    std::pair<string,int> tmp_pair (topic,topicIdCounter);
    if(topic_idByClient_map.find(topic)==topic_idByClient_map.end()) {        //checks if the topic already exists
        topic_idByClient_map.insert(tmp_pair);
        topicIdCounter++;
        return topicIdCounter - 1;                                           //returning the number of that topic
    } else {                    //already exists
        return topic_idByClient_map.find(topic)->second;            ///if we send this to the server, nothin will happen
    }
    //------------------- end edit 11/1 --------------------------
}

//unsubscribe to the topic - given ONLY the string topic
// returns the topic_id or -1 if not exists
int User::unsubtopic(string topic) {
    //------------------- start edit 11/1 ------------------------
    if(topic_idByClient_map.find(topic)!=topic_idByClient_map.end()){
        int topic_id = topic_idByClient_map.find(topic)->second;
        topic_idByClient_map.erase(topic);                                 //deleting the topic fromt he map
        return topic_id;                                                   //returning the #topicId of that topic
    }
    else{                       //not exists
        return -1;                                                 ///if we send this to the server, nothin will happen
    }
    //------------------- end edit 11/1 --------------------------
}

///Receipts functions
//returning the receiptId that has been added to the map, with
int User::addReceiptId()  {
    //------------------- start edit 11/1 ------------------------
    std::pair<int,bool> tmp_pair (receiptIdCounter, false);     //this will be a new receipt
    receiptId_returned_map.insert(tmp_pair);
    receiptIdCounter++;
    return receiptIdCounter-1;
    //------------------- end edit 11/1 --------------------------
}

//setting the receiptId to true, means that it has returned fromt he server
void User::returnedReceiptId(int receiptId) {
    //------------------- start edit 11/1 ------------------------
    receiptId_returned_map.find(receiptId)->second=true;               //setting the receipt Id to true
    //------------------- end edit 11/1 --------------------------
}

//checking the status of a specific receiptId
bool User::receiptStatus(int receiptId) {
    //------------------- start edit 11/1 ------------------------
    return receiptId_returned_map.find(receiptId)->second;
    //------------------- end edit 11/1 --------------------------

}

