/*
 * IOKernelRW.h - macOS IOKit 커널 확장을 위한 클래스 정의
 *
 * 이 헤더는 IOKit 프레임워크 기반의 커널 확장에서 사용되는
 * 서비스 클래스 IOKernelRW를 정의합니다.
 *
 * 이 클래스는 다음과 같은 역할을 합니다:
 * 
 * - IOKit의 IOService 클래스를 상속하여 macOS 드라이버로 등록됨
 * - start() 메서드를 오버라이드하여 드라이버 초기화 시 동작 정의
 *
 * 요약:
 * ┌────────────────────┬────────────────────────────────────┐
 * │ 선언 항목           │ 설명                                │
 * ├────────────────────┼────────────────────────────────────┤
 * │ class IOKernelRW    │ 커널 확장을 위한 서비스 클래스 정의   │
 * │ OSDeclareFinalStructors │ 메타클래스 및 생성자/소멸자 지원 매크로 │
 * │ bool start(IOService*)  │ 드라이버 시작 시 초기화 함수 오버라이딩 │
 * └────────────────────┴────────────────────────────────────┘
 */

#ifndef IOKERNELRW_H
#define IOKERNELRW_H

#include <IOKit/IOService.h> // IOService 클래스 포함 (IOKit 서비스의 기본 클래스)

/*
 * IOKernelRW 클래스 정의
 *
 * 이 클래스는 커널에서 로드될 KEXT(커널 확장)의 메인 서비스 클래스입니다.
 * IOService를 상속받아 IOKit 드라이버의 생명주기(lifecycle)에 따라 동작합니다.
 */
class IOKernelRW : public IOService
{
    /*
     * OSDeclareFinalStructors(IOKernelRW)
     * -------------------------------------
     * 이 매크로는 IOKit 클래스에 필요한 RTTI(type info) 및 생성자/소멸자 메타 정보를 자동 생성합니다.
     * FinalStructors는 이 클래스를 상속 불가능하게 만듭니다.
     */
    OSDeclareFinalStructors(IOKernelRW);

public:
    /*
     * start(IOService* provider)
     * ----------------------------
     * 이 메서드는 KEXT가 로드되면서 시작될 때 호출됩니다.
     * provider는 이 드라이버를 "호스팅"하거나 "상위 디바이스"로 연결된 장치입니다.
     *
     * 매개변수:
     *  - provider: 이 드라이버의 상위 디바이스(IOService 포인터)
     *
     * 반환값:
     *  - true: 드라이버 시작 성공
     *  - false: 실패 시 로드 중단됨
     */
    virtual bool start(IOService *provider) override;
};

#endif /* IOKERNELRW_H */
