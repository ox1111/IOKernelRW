/*
 * IOKernelRWUserClient.h - IOKit 기반 커널 확장의 사용자 클라이언트 정의
 *
 * 이 파일은 IOKernelRWUserClient 클래스를 정의합니다.
 * IOUserClient를 상속받아 사용자 공간 프로그램이 IOKernelRW 커널 확장과 통신할 수 있도록 지원합니다.
 *
 * 이 클래스는 다음과 같은 역할을 합니다:
 * - initWithTask()를 통해 사용자 프로세스의 접근을 초기화하고
 * - externalMethod()를 통해 사용자 요청을 처리하며
 * - 네 가지 핵심 기능(readVirt, writeVirt, readPhys, writePhys)을 외부 메서드로 제공
 *
 * 요약:
 * ┌────────────────────────────┬──────────────────────────────────────┐
 * │ 선언 항목                   │ 설명                                 │
 * ├────────────────────────────┼──────────────────────────────────────┤
 * │ initWithTask               │ 사용자 태스크로 초기화               │
 * │ externalMethod             │ 외부 메서드 디스패치 진입점          │
 * │ readVirt / writeVirt       │ 가상 메모리 읽기 / 쓰기 구현        │
 * │ readPhys / writePhys       │ 물리 메모리 읽기 / 쓰기 구현        │
 * │ physcopy                   │ 내부 물리 메모리 복사 헬퍼 함수      │
 * └────────────────────────────┴──────────────────────────────────────┘
 */

#ifndef IOKERNELRWUSERCLIENT_H
#define IOKERNELRWUSERCLIENT_H

#include <IOKit/IOUserClient.h> // IOUserClient 기반 클래스 포함

/*
 * IOKernelRWUserClient 클래스
 *
 * 이 클래스는 사용자 공간과 커널 확장 간의 연결을 담당합니다.
 * 사용자는 이 클라이언트를 통해 커널 메모리 조작 요청을 전송할 수 있습니다.
 */
class IOKernelRWUserClient : public IOUserClient
{
    /*
     * OSDeclareFinalStructors(IOKernelRWUserClient)
     * -----------------------------------------------
     * IOKit 메타클래스를 생성하며, 해당 클래스의 상속 제한(Final) 및
     * 생성자/소멸자 자동 정의를 도와주는 매크로입니다.
     */
    OSDeclareFinalStructors(IOKernelRWUserClient);

public:
    /*
     * initWithTask()
     * ----------------
     * 해당 메서드는 사용자 클라이언트가 어떤 태스크(프로세스)에 속하는지를 설정합니다.
     *
     * 파라미터:
     * - owningTask: 이 클라이언트를 생성한 사용자 프로세스의 태스크 포인터
     * - securityID: 보안 토큰이나 정책 식별자 (보통 무시됨)
     * - type: 사용자 클라이언트의 타입 식별자 (커널 모듈 구현에 따라 다름)
     *
     * 반환값:
     * - true: 초기화 성공
     * - false: 실패
     */
    virtual bool initWithTask(task_t owningTask, void *securityID, uint32_t type) override;

    /*
     * externalMethod()
     * ------------------
     * 사용자 공간에서 전달된 메서드 호출 요청을 처리합니다.
     *
     * 파라미터:
     * - selector: 호출할 메서드 ID
     * - args: 전달된 인자 구조체
     * - dispatch: 디스패치 테이블 (선택적으로 사용)
     * - target: 호출 대상 객체
     * - reference: 참조 포인터
     *
     * 반환값:
     * - IOReturn: 요청 처리 결과 (성공 또는 오류 코드)
     */
    virtual IOReturn externalMethod(uint32_t selector,
                                    IOExternalMethodArguments *args,
                                    IOExternalMethodDispatch *dispatch,
                                    OSObject *target,
                                    void *reference) override;

private:
    /*
     * readVirt()
     * ------------
     * 커널 가상 주소에서 사용자 주소로 메모리를 복사하는 외부 메서드 구현
     */
    static IOReturn readVirt(IOKernelRWUserClient *client, void *reference, IOExternalMethodArguments *args);

    /*
     * writeVirt()
     * -------------
     * 사용자 주소에서 커널 가상 주소로 메모리를 복사하는 외부 메서드 구현
     */
    static IOReturn writeVirt(IOKernelRWUserClient *client, void *reference, IOExternalMethodArguments *args);

    /*
     * readPhys()
     * ------------
     * 커널 물리 메모리에서 사용자 공간으로 메모리를 복사하는 외부 메서드 구현
     */
    static IOReturn readPhys(IOKernelRWUserClient *client, void *reference, IOExternalMethodArguments *args);

    /*
     * writePhys()
     * -------------
     * 사용자 공간에서 커널 물리 메모리로 메모리를 복사하는 외부 메서드 구현
     */
    static IOReturn writePhys(IOKernelRWUserClient *client, void *reference, IOExternalMethodArguments *args);

    /*
     * physcopy()
     * ------------
     * 물리 주소 간 복사를 수행하는 내부 헬퍼 함수
     *
     * 파라미터:
     * - src: 소스 물리 주소
     * - dst: 목적지 물리 주소
     * - len: 복사할 바이트 수
     * - alignment: 정렬 크기 (0, 4, 8)
     * - direction: 읽기/쓰기 방향 (IODirection enum 값)
     */
    static IOReturn physcopy(uint64_t src, uint64_t dst, uint64_t len, uint64_t alignment, IODirection direction);
};

#endif /* IOKERNELRWUSERCLIENT_H */
