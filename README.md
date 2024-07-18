# QingLiaoChatClient
This is a chat client.  
If you want the server, see [QingLiaoChatServer](https://github.com/Hcolda/QingLiaoChatServer)

## Current Progress
![img](./doc/img.png)

## Build
### Essential tools
1. [vcpkg](https://github.com/microsoft/vcpkg)

### Build with cmake
```cmd
vcpkg install asio
vcpkg install qt

cmake -S . -B build
cmake --build build --config Release
```

## TODO
- [x] Network (with TLS-1.2)
- [x] Login and register
- [ ] Add friends and groups
- [ ] Room (private room and group room)
- [ ] Voice chat
- [ ] File transport
