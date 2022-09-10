

class Encoder{
    private:
        int primaryPin;
        int secondaryPin;
        bool lastState;
        int position;
        int min = -1;
        int max = -1;
    public:
        Encoder(int primaryPin, int secondaryPin);
        Encoder(int primaryPin, int secondaryPin, int min, int max);
        bool check();
        int getPosition();
        void setPosition(int position);
};