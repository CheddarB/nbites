/**
 * File parser implementation
 * @author Octavian Neamtu
 */

#pragma once

#include <unistd.h>
#include <iostream>
#include <string>
#include <google/protobuf/message.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "Parser.h"

namespace memory {

namespace log {

namespace proto_io = google::protobuf::io;
namespace proto = google::protobuf;

class FileParser : Parser <proto::Message>{

public:
    FileParser(boost::shared_ptr<proto::Message> message,
               const char* _file_name);
    FileParser(boost::shared_ptr<proto::Message> message,
               int _file_descriptor);

    ~FileParser();

    void initStreams();

    const LogHeader getHeader();
    boost::shared_ptr<const proto::Message> getNextMessage();
    boost::shared_ptr<const proto::Message> getPrevMessage();

private:
    void readHeader();

private:
    int file_descriptor;
    int current_size;

    bool finished;

    proto_io::FileInputStream* raw_input;
    proto_io::CodedInputStream* coded_input;

};

}
}
