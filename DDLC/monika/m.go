package main
import (
 "os"
 "golang.org/x/image/bmp"
)
func main() {
 out , err := os.OpenFile("monika.bin", os.O_RDWR|os.O_CREATE, 0755)
 if err != nil {
  panic(err)
 }
 defer out.Close()
 file, err := os.Open("monika.bmp")
 if err != nil {
  panic(err)
 }
 defer file.Close()
img,  _ := bmp.Decode(file)
 size    := img.Bounds()
b := byte(0)
 l := 0
// iterate rows
 for y := size.Min.Y; y < size.Max.Y; y ++ {
  for x := size.Min.X; x < size.Max.X; x ++ {
   r, _, _, _ := img.At(x, y).RGBA()
   if r == 0 {
    l ++
    b = b << 1
   } else if r == 0xffff {
    l ++
    b = (b << 1) + 1
   } else {
    panic("found neither 0x0000 nor 0xffff")
   }
   // write one byte
   if l >= 8 {
    out.Write([]byte{b})
    l = 0
    b = byte(0)
   }
  }
 }
}