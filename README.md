# multi-chat
multi-chat server and client / Linux / multi-thread using mutex

기능 : 귓속말 기능 / 사용자 접속 및 퇴장 시 알림 / 채팅

<img src="https://user-images.githubusercontent.com/76546008/200307284-1ad3aec4-d079-4adb-8b5b-2d86dc0bf1c6.png" width = 640/>

< 시뮬레이션 시나리오 >

1. 서버 오픈
1. 전자통신 사용자 접속
1. 16학번 사용자 접속
1. 조시언 사용자 접속
1. [채팅] 조시언: 안녕하세요!
1. [귓속말] 16학번 → 조시언: 반갑습니다~
1. [귓속말] 전자통신 → 조시언: 오랜만이네요
1. @exit 전자통신 사용자 종료
1. @show 조시언
1. @exit 16학번 사용자 종료
1. @show 조시언
1. @exit 조시언 사용자 종료

내용
- 고정 포트번호 : 3500
- 멀티 쓰레드 방식
- 로컬 동작