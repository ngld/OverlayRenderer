#include <thread>
#include "packets.h"
#include "logging.h"
#include "win_pipe_wrapper.h"

using namespace OverlayRenderer::Common;

WinPipeWrapper::WinPipeWrapper(std::string name) {
    _name = "\\\\.\\pipe\\" + name;
}

WinPipeWrapper::~WinPipeWrapper() {
};

bool WinPipeWrapper::Listen() {
    _server = true;
    _listen_thread = std::thread(&WinPipeWrapper::_Listen, this);
    return true;
}

void WinPipeWrapper::_Listen() {
    LOG(INFO) << _name << ": Waiting for client to connect...";
    while (true) {
        auto new_pipe = CreateNamedPipe(
            _name.c_str(),
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
            10,
            10 * 1024 * 1024,
            1024 * 1024,
            300,
            nullptr
        );

        if (new_pipe == INVALID_HANDLE_VALUE) {
            new_pipe = nullptr;
            _error = "Failed to create pipe!";

            LOG(ERROR) << _name << ": " << _error << " " << GetLastError();
            return;
        }

        auto result = ConnectNamedPipe(new_pipe, nullptr);

        if (result || GetLastError() == ERROR_PIPE_CONNECTED) {
            COMMTIMEOUTS timeouts;
            timeouts.ReadIntervalTimeout = 1000;
            timeouts.ReadTotalTimeoutConstant = 1000;
            timeouts.WriteTotalTimeoutConstant = 100;
            SetCommTimeouts(new_pipe, &timeouts);
            _pipe = new_pipe;

            continue;
        } else {
            char* errorMsg;
            DWORD_PTR args[] = { NULL };
            FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |\
                    FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                0, GetLastError(), 0, (char*) &errorMsg, 0, (va_list*) args
            );

            LOG(ERROR) << _name << ": Connection failed: " << errorMsg;
            free(errorMsg);
        }
    }
}

bool WinPipeWrapper::MaybeConnect() {
    if (_pipe == nullptr)
        return Connect();

    return true;
}

bool WinPipeWrapper::Connect() {
    _server = false;
    _pipe = CreateFile(
        _name.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (_pipe == INVALID_HANDLE_VALUE) {
        _pipe = nullptr;
        _error = "Failed to open pipe! ";

        LOG(ERROR) << _name << ": " << _error << GetLastError();
        return false;
    }

    COMMTIMEOUTS timeouts;
    timeouts.ReadIntervalTimeout = 1000;
    timeouts.ReadTotalTimeoutConstant = 1000;
    timeouts.WriteTotalTimeoutConstant = 500;
    SetCommTimeouts(_pipe, &timeouts);

    return true;
}

void WinPipeWrapper::EncodePacket(void** buffer, int& length, const Packet& pkt) {
    length = pkt.ByteSizeLong() + sizeof(((PacketWrapper*) 0)->size);
    PacketWrapper* wrapper = (PacketWrapper*) malloc(length);
    
    wrapper->size = pkt.ByteSizeLong();
    pkt.SerializeToArray((void*) &wrapper->buffer, length);

    *buffer = wrapper;
}

bool WinPipeWrapper::SendRaw(void* buffer, int length, unsigned long* bytesWritten) {
    bool result = WriteFile(_pipe, buffer, length, bytesWritten, nullptr);
    auto error = GetLastError();

    if (!result) {
        if (error == ERROR_INVALID_HANDLE || error == ERROR_PIPE_NOT_CONNECTED) {
            LOG(ERROR) << _name << ": Connection lost";
            _pipe = nullptr;
            return false;
        }

        LOG(ERROR) << _name << ": Unexpected error during write: " << GetLastError();
        return false;
    }

    return true;
}

bool WinPipeWrapper::Send(const Packet& pkt) {
    void *buffer;
    unsigned long bytesWritten;
    int length;

    if (_pipe == nullptr)
        return false;

    length = pkt.ByteSizeLong();

    //EncodePacket(&buffer, length, pkt);
    if (!SendRaw(&length, sizeof(length), &bytesWritten))
        return false;

    buffer = malloc(length);
    pkt.SerializeToArray(buffer, length);

    if (!SendRaw(buffer, length, &bytesWritten)) {
        free(buffer);
        return false;
    }

    free(buffer);
    return true;
}

bool WinPipeWrapper::Receive(Packet& pkt) {
    int length;
    unsigned long bytesRead;

    while (_pipe == nullptr) {
        Sleep(1000);
    }

    bool result = ReadFile(_pipe, &length, sizeof(length), &bytesRead, nullptr);

    if (!result) {
        auto error = GetLastError();
        if (error == ERROR_INVALID_HANDLE || error == ERROR_BROKEN_PIPE) {
            _pipe = nullptr;
            LOG(ERROR) << _name << ": Connection lost";
            return false;
        }

        if (error != 535) {
            LOG(ERROR) << _name << ": Unexpected error during read: " << error;

            DisconnectNamedPipe(_pipe);
            _pipe = nullptr;
        }
        return false;
    }

    if (bytesRead < sizeof(length)) {
        LOG(ERROR) << _name << ": Incomplete read: " << GetLastError();
        return false;
    }

    if (length > 100 * 1024 * 1024) {
        LOG(ERROR) << _name << ": Skipping humungous frame: " << length;
        return false;
    }

    if (length < 0) {
        LOG(ERROR) << _name << ": Frame size can't be negative: " << length;
        return false;
    }

    void *buffer = malloc(length);
    result = ReadFile(_pipe, buffer, length, &bytesRead, nullptr);

    if (!result) {
        LOG(ERROR) << _name << ": Unexpected error during read: " << GetLastError();
        free(buffer);
        return false;
    }

    if (bytesRead < length) {
        LOG(ERROR) << _name << ": Incomplete packet read: " << GetLastError() << " (" << bytesRead << " < " << length << ")";
        free(buffer);
        return false;
    }

    pkt.ParseFromArray(buffer, length);
    free(buffer);
    return true;
}
