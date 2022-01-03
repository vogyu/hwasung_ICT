#파이썬 씨발마지막코드
import tensorflow.keras
import cv2  # opencv
import numpy as np  # 영상을 다루기 위해 배열 numpy
import serial  # 시리얼 통신을 위함
import os  # 운영체제 제어를 위함
from socket import *  # 소켓통신을 위함
from threading import *  # object detection과 통신을 위해 쓰레딩
import time

# 머신러닝 전처리 시작
# 모델 위치
model_filename = 'PET\\keras_model.h5'

# 케라스 모델 가져오기
model = tensorflow.keras.models.load_model(model_filename)

# 카메라를 제어할 수 있는 객체
capture = cv2.VideoCapture(0)

# 카메라 길이 너비 조절
capture.set(cv2.CAP_PROP_FRAME_WIDTH, 320)
capture.set(cv2.CAP_PROP_FRAME_HEIGHT, 240)
# 끝

PORT = 'COM6'  # 아두이노 포트
host = "172.16.3.91"  # host ip #"192.168.0.213"
Port = 54321  # 시리얼 통신에 대한 초기설정
SER = serial.Serial(PORT, 9600)

cam_on_flag = 3
serverSocket = socket(AF_INET, SOCK_STREAM)
serverSocket.bind((host, Port))
serverSocket.listen(1)
print("wait")
(connectionSocket, addr) = serverSocket.accept()
print(str(addr), "from connect.")  # connect check


# server setting #
# 이미지 처리하기

def preprocessing(frame):
    # frame_fliped = cv2.flip(frame, 1)
    # 사이즈 조정 티쳐블 머신에서 사용한 이미지 사이즈로 변경해준다.
    size = (224, 224)
    frame_resized = cv2.resize(frame, size, interpolation=cv2.INTER_AREA)

    # 이미지 정규화
    # astype : 속성+-
    frame_normalized = (frame_resized.astype(np.float32) / 127.0) - 1

    # 이미지 차원 재조정 - 예측을 위해 reshape 해줍니다.
    # keras 모델에 공급할 올바른 모양의 배열 생성
    frame_reshaped = frame_normalized.reshape((1, 224, 224, 3))
    # print(frame_reshaped)
    return frame_reshaped


pet_kind = 0
pet_str = ""


# 예측용 함수
def predict(frame):
    prediction = model.predict(frame)
    return prediction



max_flag=0
def camfunc():
    cap = cv2.VideoCapture(0)
    while (True):
        if SER.readable():
            cap = cv2.VideoCapture(0)
            res = SER.readline()
            print(res.decode()[:len(res) - 1])
            cam_on_flag = int(res.decode()[:len(res) - 1])
        if cam_on_flag == 5:
            ret, frame = cap.read()
            if not ret:
                print("error!")
                break
            cv2.imshow("VideoFrame", frame)
            preprocessed = preprocessing(frame)
            prediction = predict(preprocessed)
            print(prediction)
            if prediction[0, 0] > prediction[0, 1] and prediction[0, 0] > prediction[0, 2] and prediction[0, 0] > \
                    prediction[0, 3]:  # 무색 무라벨
                cap.release()
                cv2.destroyAllWindows()
                SER.write([2])
                cam_on_flag = 0
                message = (str("No label clear PET"))  # message를 입력해서 보내줌
                connectionSocket.send(message.encode())  # messagem를 보낼때에는  encode함수 사용
            elif prediction[0, 1] > prediction[0, 0] and prediction[0, 1] > prediction[0, 2] and prediction[0, 1] > \
                    prediction[0, 3]:  # 무색 유라벨
                cap.release()
                cv2.destroyAllWindows()
                SER.write([1])
                cam_on_flag = 0
                message = (str("Label  clear PET"))  # message를 입력해서 보내줌
                connectionSocket.send(message.encode())  # messagem를 보낼때에는  encode함수 사용
            elif prediction[0, 2] > prediction[0, 0] and prediction[0, 2] > prediction[0, 1] and prediction[0, 2] > \
                    prediction[0, 3]:  # 유색
                cap.release()
                cv2.destroyAllWindows()
                SER.write([3])
                cam_on_flag = 0
                message = (str("Color PET"))  # message를 입력해서 보내줌
                connectionSocket.send(message.encode())  # messagem를 보낼때에는  encode함수 사용
            else:
                message = (str("NOTHING"))  # message를 입력해서 보내줌
                connectionSocket.send(message.encode())  # messagem를 보낼때에는  encode함수 사용

        elif cam_on_flag == 0:
            print('대기하시오.')
            # cap = cv2.VideoCapture(0)
    cap.release()
    cv2.destroyAllWindows()


def serverfunc():
    # 스레드를 사용하여 소켓통신을 하기 위해 만든 함수이다.

    # host에 대한 ip와 해당하는 포트 번호를 열어주었다. -> 포트포워딩을 하였고, 이 코드는 서버 코드이다.
    ## server setting ##
    # 휴대폰 어플을 통해 접속하여, 해당하는 버튼을 눌렀을 경우, 그에 맞는 신호를 아두이노에 보내주는 반복문 + 조건문들 이다.
    while (True):
        # if data=='d':
        #   print("recv data:", data.decode("utf-8"))
        # print("recv data:", data.decode("utf-8"))z
        # connectionSocket.sendall("hello")

        # message = input(str("plz enter your msg :")) #message를 입력해서 보내줌
        # connectionSocket.send(message.encode())      #messagem를 보낼때에는  encode함수 사용

        data = connectionSocket.recv(1024)
        if data.decode("utf-8") == 'm':  # manual mode on
            SER.write([20])  # auto mode serial number send
            print("recv data:", data.decode("utf-8"))
        elif data.decode("utf-8") == 'a':  # auto mode on
            SER.write([10])  # manual mode on
            print("recv data:", data.decode("utf-8"))

        # xd (x축 버튼 down) xu(x축 버튼 up), xcd(x축 반시계방향 버튼 down), xcu(x축 반시계방향 버튼 up)##
        elif data.decode("utf-8") == 'xd':  # xcw x축 시계방향 버튼이 눌렸을 때
            print("recv data:", data.decode("utf-8"))  # XU 일 경우 모터계속 구동
            SER.write([11])  # right turn serial number '1'
        elif data.decode("utf-8") == 'xu':  # xcw x축 시계방향 버튼이 떼질때
            print("recv data:", data.decode("utf-8"))
            SER.write([4])  # 'all stop' serial number  '4'
        elif data.decode("utf-8") == 'xcd':  # xccw x축 반시계 방향 버튼이 눌렀을떄
            print("recv data:", data.decode("utf-8"))
            SER.write([12])  # backward serial number '2'
        elif data.decode("utf-8") == 'xcu':  # xccw멈춤, x축 반시계 방향 버튼이 떼졌을때
            print("recv data:", data.decode("utf-8"))
            SER.write([4])  # 'all stop' serial number  '4'
        # -------------------------------------------------------------------#
        elif data.decode("utf-8") == 'yd':  # ycw  y축 시계방향 버튼이 눌렸을 때
            print("recv data:", data.decode("utf-8"))
            SER.write([21])  # auto mode serial number '21'
        elif data.decode("utf-8") == 'yu':  # ycw멈춤
            print("recv data:", data.decode("utf-8"))
            SER.write([4])  # 'all stop' serial number  '4'
        elif data.decode("utf-8") == 'ycd':  # yccw
            print("recv data:", data.decode("utf-8"))
            SER.write([22])  # stop mode serial  mode number '22'
        elif data.decode("utf-8") == 'ycu':  # yccw멈춤
            print("recv data:", data.decode("utf-8"))
            SER.write([4])  # 'all stop' serial number  '4'
        # -------------------------------------------------------------------#
        elif data.decode("utf-8") == 'zud':  # z축이 up버튼 눌렸을때
            print("recv data:", data.decode("utf-8"))
            SER.write([31])  # spray run serial number '10'
        elif data.decode("utf-8") == 'zuu':  # z축 up버튼이 떼졌을때
            print("recv data:", data.decode("utf-8"))
            SER.write([4])  # 'all stop' serial number  '4'
        elif data.decode("utf-8") == 'zdd':  # z축 down 버튼이 눌렸을때
            print("recv data:", data.decode("utf-8"))
            SER.write([32])  # spray run serial number '10'
        elif data.decode("utf-8") == 'zdu':  # z축 down 버튼이 떼졌을때
            print("recv data:", data.decode("utf-8"))
            SER.write([4])  # 'all stop' serial number  '4'
        # -------------------------------------------------------------------#
        elif data.decode("utf-8") == 'wd':  # gripper close 버튼이 떼졌을 때
            print("recv data:", data.decode("utf-8"))
            SER.write([41])  # spray run serial number '10'
        elif data.decode("utf-8") == 'wu':  # gripper close 버튼이 떼졌을 때
            print("recv data:", data.decode("utf-8"))
            SER.write([4])  # 'all stop' serial number  '4'
        elif data.decode("utf-8") == 'wcd':  # gripper close 버튼이 떼졌을 때
            print("recv data:", data.decode("utf-8"))
            SER.write([42])  # spray run serial number '10'
        elif data.decode("utf-8") == 'wcu':  # gripper close 버튼이 떼졌을 때
            print("recv data:", data.decode("utf-8"))
            SER.write([4])  # 'all stop' serial number  '4'
        # -------------------------------------------------------------------#
        elif data.decode("utf-8") == 'god':  # gripper  open 버튼이 눌렸을 때
            print("recv data:", data.decode("utf-8"))
            SER.write([51])  # spray run serial number '10'
        elif data.decode("utf-8") == 'gou':  # gripper  open 버튼이 떼졌을 때
            print("recv data:", data.decode("utf-8"))
            SER.write([4])  # 'all stop' serial number  '4'
        elif data.decode("utf-8") == 'gcd':  # gripper close 버튼이 눌렸을 때
            print("recv data:", data.decode("utf-8"))
            SER.write([52])  # spray run serial number '10'
        elif data.decode("utf-8") == 'gcu':  # gripper close 버튼이 떼졌을 때
            print("recv data:", data.decode("utf-8"))
            SER.write([4])  # 'all stop' serial number  '4'
        # -------------------------------------------------------------------#
        elif data.decode("utf-8") == 'stop':  # 비상 정지 버튼 -> break
            print("recv data:", data.decode("utf-8"))
            SER.write([98])  # Arduino roop break serial num '98'

    serverSocket.close()


# 스레드를 실행시키는 문장
if __name__ == '__main__':
    proc = Thread(target=camfunc, args=())
    proc2 = Thread(target=serverfunc, args=())
    proc.start()
    proc2.start()