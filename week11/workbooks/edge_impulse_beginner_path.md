# 선수학습: CSV 로깅에서 Edge Impulse 기초까지

> 11주차의 필수 목표는 SPI microSD CSV 로깅입니다. Edge Impulse는 별도 확장 학습입니다.

## 1. 왜 학습하나요?
- GPIO/센서 측정 → SD에 CSV 저장 → 데이터 분석 → 데이터셋/라벨 → 머신러닝 → ESP32 추론.
- Edge Impulse는 모델 학습·최적화·배포를 돕는 도구이며 SPI 신호의 impulse와 다릅니다.

## 2. Windows 11 기초
1. microSD의 `dht11.csv`를 Excel에서 열어 온도·습도 변화 그래프를 만든다.
2. 결측/중복/센서 오류를 확인한다.
3. 규칙 기반 판단(`temperature >= 30`)과 학습 기반 판단의 차이를 설명한다.
4. https://studio.edgeimpulse.com 에 접속하여 프로젝트를 만든다.
5. 입력과 정답 레이블의 의미 및 Training/Test 데이터 분할 원리를 확인한다.
6. 공식 문서의 초보자 예제로 Studio의 Data acquisition → Impulse design → Training → Model testing → Deployment 흐름을 따라간다.

## 3. 데이터 준비 주의점
기존 `dht11.csv`는 일반 로그 파일로서, Edge Impulse의 선택한 프로젝트 입력 타입이 요구하는 학습 데이터 형식과 다를 수 있다. 센서 데이터 형식/윈도우/라벨을 정의한 후 지원 업로드 형식으로 변환해야 한다. 학생이 임의로 구분한 정상/이상 라벨만으로 높은 정확도를 주장하지 않는다. 같은 측정 구간에서 만든 훈련/시험 샘플 중복을 피한다.

## 4. ESP32 배포
Studio에서 타깃 라이브러리와 지원 가능한 모델 크기를 확인한다. ESP32-DevKitC V4, Arduino 코어, RAM/Flash, 라이브러리 호환성을 확인한 후 실물 빌드·추론을 검증한다. 이 워크북에는 실물 검증이 완료되었다는 뜻이 아니다.

## 5. 학생 체크리스트
- [ ] CSV 12행 이상 수집
- [ ] Excel 그래프 2개 작성
- [ ] 입력·레이블·훈련·시험 정의 설명
- [ ] Edge Impulse 프로젝트 개설
- [ ] 데이터 유형에 맞는 예제 데이터 등록
- [ ] Impulse 구성 확인
- [ ] 학습·시험 결과 해석
- [ ] 배포 형식과 보드 호환성 확인

참고: https://docs.edgeimpulse.com/docs
노션: https://app.notion.com/p/41cb2e6bf807814db680fa67020bbcd8
