public class HelloWorld {
    private String message = "Hello, World!";

    public void printMessage() {
        System.out.println(message);
    }

    public static void main(String[] args) {
        HelloWorld hw = new HelloWorld();
        hw.printMessage();

        int x = 5;
        int y = 10;
        int sum = x + y;

        if (sum > 10) {
            System.out.println("Sum is greater than 10");
        }

        return;
    }
}
