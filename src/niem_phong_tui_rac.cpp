#include <Arduino.h>
#include <ESP32Servo.h>

#include "niem_phong_tui_rac.h"

// Anh xa chan ket noi
const int CHAN_CAM_BIEN_TAY = 19;
const int CHAN_NUT_NIEM_PHONG = 25;
const int CHAN_SERVO_NAP = 13;
const int CHAN_RELAY_MAYSO = 14;

// Cac moc thoi gian cua he thong
const unsigned long THOI_GIAN_DONG_NAP_MS = 4000;
const unsigned long THOI_GIAN_CHO_SERVO_MS = 1000;
const unsigned long THOI_GIAN_GIA_NHIET_MS = 3500;
const unsigned long THOI_GIAN_LAM_NGUOI_MS = 2000;
const unsigned long THOI_GIAN_CHONG_DOI_NUT_MS = 50;

enum class TrangThaiNiemPhong {
  CHO_LENH,
  DANG_DONG_NAP,
  DANG_GIA_NHIET,
  DANG_LAM_NGUOI
};

Servo servoNap;
TrangThaiNiemPhong trangThaiNiemPhong = TrangThaiNiemPhong::CHO_LENH;

unsigned long thoiDiemPhatHienTayCuoi = 0;
unsigned long thoiDiemBatDauTrangThai = 0;
unsigned long thoiDiemNutThayDoi = 0;

bool napDangMo = false;
bool giaTriNutLanTruoc = HIGH;
bool trangThaiNutOnDinh = HIGH;

void moNap() {
  servoNap.write(90);
  napDangMo = true;
}

void dongNap() {
  servoNap.write(0);
  napDangMo = false;
}

void batDauNiemPhong(unsigned long thoiGianHienTai) {
  // Khoa lien dong: luon tat mayso va dong nap truoc khi gia nhiet.
  digitalWrite(CHAN_RELAY_MAYSO, LOW);
  dongNap();
  trangThaiNiemPhong = TrangThaiNiemPhong::DANG_DONG_NAP;
  thoiDiemBatDauTrangThai = thoiGianHienTai;
  Serial.println("Bat dau niem phong: dang dong nap...");
}

void capNhatNutNiemPhong(unsigned long thoiGianHienTai) {
  bool giaTriDangDoc = digitalRead(CHAN_NUT_NIEM_PHONG);

  if (giaTriDangDoc != giaTriNutLanTruoc) {
    thoiDiemNutThayDoi = thoiGianHienTai;
    giaTriNutLanTruoc = giaTriDangDoc;
  }

  if ((thoiGianHienTai - thoiDiemNutThayDoi >= THOI_GIAN_CHONG_DOI_NUT_MS) &&
      (giaTriDangDoc != trangThaiNutOnDinh)) {
    trangThaiNutOnDinh = giaTriDangDoc;
    if (trangThaiNutOnDinh == LOW &&
        trangThaiNiemPhong == TrangThaiNiemPhong::CHO_LENH) {
      batDauNiemPhong(thoiGianHienTai);
    }
  }
}

void capNhatQuyTrinhNiemPhong(unsigned long thoiGianHienTai) {
  switch (trangThaiNiemPhong) {
    case TrangThaiNiemPhong::CHO_LENH:
      return;

    case TrangThaiNiemPhong::DANG_DONG_NAP:
      if (thoiGianHienTai - thoiDiemBatDauTrangThai >=
          THOI_GIAN_CHO_SERVO_MS) {
        digitalWrite(CHAN_RELAY_MAYSO, HIGH);
        trangThaiNiemPhong = TrangThaiNiemPhong::DANG_GIA_NHIET;
        thoiDiemBatDauTrangThai = thoiGianHienTai;
        Serial.println("Nap da dong -> Bat mayso trong 3.5 giay");
      }
      break;

    case TrangThaiNiemPhong::DANG_GIA_NHIET:
      if (thoiGianHienTai - thoiDiemBatDauTrangThai >=
          THOI_GIAN_GIA_NHIET_MS) {
        digitalWrite(CHAN_RELAY_MAYSO, LOW);
        trangThaiNiemPhong = TrangThaiNiemPhong::DANG_LAM_NGUOI;
        thoiDiemBatDauTrangThai = thoiGianHienTai;
        Serial.println("Da tat mayso -> Cho nguoi trong 2 giay");
      }
      break;

    case TrangThaiNiemPhong::DANG_LAM_NGUOI:
      if (thoiGianHienTai - thoiDiemBatDauTrangThai >=
          THOI_GIAN_LAM_NGUOI_MS) {
        // Chi mo nap sau khi mayso da tat va moi han da nguoi.
        digitalWrite(CHAN_RELAY_MAYSO, LOW);
        moNap();
        thoiDiemPhatHienTayCuoi = thoiGianHienTai;
        trangThaiNiemPhong = TrangThaiNiemPhong::CHO_LENH;
        Serial.println("Niem phong hoan tat -> Mo nap de lay tui rac");
      }
      break;
  }
}

void capNhatNapTuDong(unsigned long thoiGianHienTai) {
  // Bo qua cam bien tay khi quy trinh niem phong dang hoat dong.
  if (trangThaiNiemPhong != TrangThaiNiemPhong::CHO_LENH) {
    return;
  }

  int trangThaiCamBien = digitalRead(CHAN_CAM_BIEN_TAY);
  if (trangThaiCamBien == LOW) {
    if (!napDangMo) {
      moNap();
      Serial.println("Phat hien tay -> Mo nap");
    }
    thoiDiemPhatHienTayCuoi = thoiGianHienTai;
  } else if (napDangMo &&
             thoiGianHienTai - thoiDiemPhatHienTayCuoi >=
                 THOI_GIAN_DONG_NAP_MS) {
    dongNap();
    Serial.println("Da qua 4s khong co tay -> Dong nap");
  }
}

void khoiTaoHeThong() {
  Serial.begin(115200);

  pinMode(CHAN_CAM_BIEN_TAY, INPUT_PULLUP);
  pinMode(CHAN_NUT_NIEM_PHONG, INPUT_PULLUP);
  pinMode(CHAN_RELAY_MAYSO, OUTPUT);

  // Mayso phai luon tat khi he thong khoi dong.
  digitalWrite(CHAN_RELAY_MAYSO, LOW);

  servoNap.attach(CHAN_SERVO_NAP);
  dongNap();

  Serial.println("He thong da khoi dong!");
  Serial.println("Nhan nut GPIO 25 de bat dau niem phong tui rac.");
}

void capNhatHeThong() {
  unsigned long thoiGianHienTai = millis();
  capNhatNutNiemPhong(thoiGianHienTai);
  capNhatQuyTrinhNiemPhong(thoiGianHienTai);
  capNhatNapTuDong(thoiGianHienTai);
  delay(10);
}
