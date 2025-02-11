package main

import (
	"fmt"
	"github.com/rivo/tview"
	"net"
	"os"
)

var inputText string = "[orange]Waiting for server message..." // Default text while waiting

func main() {
	//TCP server logic
	conn, err := net.Dial("tcp", "127.0.0.1:8080")
	if err != nil {
		fmt.Println("Error connecting to server:", err)
		os.Exit(1)
	}
	defer conn.Close()

	buf := make([]byte, 1024)
	n, err := conn.Read(buf)
	if err != nil {
		fmt.Println("Error reading from server:", err)
		os.Exit(1)
	}


	//App Logic
	inputText = fmt.Sprintf("[orange]%s", string(buf[:n]))

	app := tview.NewApplication()

	textView := tview.NewTextView().
		SetText(inputText).
		SetTextAlign(tview.AlignLeft).
		SetDynamicColors(true)

	flex := tview.NewFlex().
		SetDirection(tview.FlexRow).
		AddItem(textView, 0, 1, true)

	flex.SetBorder(true)

	if err := app.SetRoot(flex, true).Run(); err != nil {
		panic(err)
	}
}
