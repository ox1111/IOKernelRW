/*
 * libiokernelrw.h - macOS용 커널 메모리 읽기/쓰기 커널 확장을 위한 사용자 공간 헬퍼
 *
 * 이 라이브러리는 커널에서 제공하는 IOKernelRW 서비스에 접근하여 다음과 같은 기능을 수행할 수 있습니다:
 *
 * 1. 커널 가상 메모리 읽기
 * 2. 커널 가상 메모리 쓰기
 * 3. 물리 메모리 읽기
 * 4. 물리 메모리 쓰기
 *
 * 해당 함수들은 IOConnectCallScalarMethod를 이용하여 커널 확장의 외부 메서드를 호출하며,
 * 각 메서드는 scalar 인자만을 받으며 반환값 외에는 출력이 없습니다.
 *
 * 요구사항:
 * - 해당 KEXT가 로드되어 있어야 하며
 * - 접근하려는 바이너리는 entitlement: com.apple.security.siguza.kernel-rw 를 포함해야 합니다
 */

#ifndef LIBIOKERNELRW_H
#define LIBIOKERNELRW_H

#include <mach/mach.h>           // Mach 커널 인터페이스 사용
#include <IOKit/IOKitLib.h>      // IOKit의 서비스 검색, 열기 함수들 제공

/*
 * iokernelrw_open()
 * ------------------
 * IOKernelRW라는 이름의 커널 서비스를 찾아 열고 연결된 포트를 반환합니다.
 * 
 * 반환값:
 * - 성공 시: io_connect_t 타입의 유저클라이언트 포트
 * - 실패 시: MACH_PORT_NULL
 */
static inline io_connect_t iokernelrw_open(void)
{
    // 서비스 이름이 "IOKernelRW"인 객체 검색
    io_service_t service = IOServiceGetMatchingService(kIOMainPortDefault, IOServiceMatching("IOKernelRW"));

    // 유효하지 않으면 실패
    if(!MACH_PORT_VALID(service))
    {
        return MACH_PORT_NULL;
    }

    io_connect_t client = MACH_PORT_NULL;

    // 현재 프로세스(mach_task_self())가 해당 서비스에 유저클라이언트를 열도록 요청
    kern_return_t ret = IOServiceOpen(service, mach_task_self(), 0, &client);

    // 서비스 객체 해제 (참조 횟수 감소)
    IOObjectRelease(service);

    // 연결 실패 시
    if(ret != KERN_SUCCESS)
    {
        return MACH_PORT_NULL;
    }

    // 성공적으로 연결된 포트 반환
    return client;
}

/*
 * iokernelrw_read()
 * ------------------
 * 커널 가상 주소 영역에서 사용자 메모리 영역으로 데이터를 복사합니다.
 *
 * 파라미터:
 * - client: IOKernelRW 유저클라이언트 포트
 * - from: 커널 가상 주소 (복사를 시작할 주소)
 * - to: 사용자 공간 포인터 (복사 대상)
 * - len: 복사할 바이트 수
 *
 * 반환값:
 * - KERN_SUCCESS 또는 오류 코드
 */
static inline kern_return_t iokernelrw_read(io_connect_t client, uint64_t from, void *to, uint64_t len)
{
    uint64_t in[] = {
        from,            // Arg0: 커널 가상 주소
        (uint64_t)to,    // Arg1: 사용자 공간 주소 (형변환 필요)
        len              // Arg2: 읽을 바이트 수
    };

    return IOConnectCallScalarMethod(
        client,          // 유저클라이언트 포트
        0,               // 메서드 ID 0: readVirt
        in, 3,           // 입력 인자 3개
        NULL, NULL       // 출력 없음
    );
}

/*
 * iokernelrw_write()
 * -------------------
 * 사용자 메모리의 데이터를 커널 가상 메모리 주소로 복사합니다.
 *
 * 파라미터:
 * - client: 유저클라이언트 포트
 * - from: 사용자 공간 주소 (복사할 원본)
 * - to: 커널 가상 주소 (대상)
 * - len: 복사할 바이트 수
 */
static inline kern_return_t iokernelrw_write(io_connect_t client, void *from, uint64_t to, uint64_t len)
{
    uint64_t in[] = {
        (uint64_t)from,  // Arg0: 사용자 공간 주소
        to,              // Arg1: 커널 가상 주소
        len              // Arg2: 쓰기 바이트 수
    };

    return IOConnectCallScalarMethod(client, 1, in, 3, NULL, NULL);  // 메서드 ID 1: writeVirt
}

/*
 * iokernelrw_read_phys()
 * ------------------------
 * 커널 물리 메모리 영역에서 사용자 공간으로 데이터를 읽어옵니다.
 *
 * 파라미터:
 * - client: 유저클라이언트 포트
 * - from: 물리 주소 (복사 시작점)
 * - to: 사용자 공간 주소 (복사 대상)
 * - len: 복사할 바이트 수
 * - align: 정렬 단위 (4 or 8은 비캐시 접근, 0은 캐시 가능)
 */
static inline kern_return_t iokernelrw_read_phys(io_connect_t client, uint64_t from, void *to, uint64_t len, uint8_t align)
{
    uint64_t in[] = {
        from,            // Arg0: 물리 주소 시작점
        (uint64_t)to,    // Arg1: 사용자 공간 주소
        len,             // Arg2: 복사 크기
        align            // Arg3: 정렬 단위
    };

    return IOConnectCallScalarMethod(client, 2, in, 4, NULL, NULL);  // 메서드 ID 2: readPhys
}

/*
 * iokernelrw_write_phys()
 * -------------------------
 * 사용자 메모리에서 커널 물리 메모리로 데이터를 복사합니다.
 *
 * 파라미터:
 * - client: 유저클라이언트 포트
 * - from: 사용자 주소 (복사 원본)
 * - to: 커널 물리 주소 (복사 대상)
 * - len: 바이트 수
 * - align: 정렬 단위 (0, 4, 8)
 */
static inline kern_return_t iokernelrw_write_phys(io_connect_t client, void *from, uint64_t to, uint64_t len, uint8_t align)
{
    uint64_t in[] = {
        (uint64_t)from,  // Arg0: 사용자 공간 주소
        to,              // Arg1: 물리 주소
        len,             // Arg2: 크기
        align            // Arg3: 정렬
    };

    return IOConnectCallScalarMethod(client, 3, in, 4, NULL, NULL);  // 메서드 ID 3: writePhys
}

#endif /* LIBIOKERNELRW_H */
