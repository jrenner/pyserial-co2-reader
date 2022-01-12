from sqlalchemy import create_engine, Column, Integer, String
from sqlalchemy.orm import declarative_base, sessionmaker
from datetime import datetime
from logzero import logger

engine = create_engine("sqlite:///database.sqlite", echo=True)

Base = declarative_base()

class Reading(Base):
    __tablename__ = 'reading'

    id = Column(Integer, primary_key=True)
    datetime = Column(String)
    co2 = Column(Integer)
    tvoc = Column(Integer)

Base.metadata.create_all(engine)
Session = sessionmaker()
Session.configure(bind=engine)

def save_reading(reading):
    try:
        dt = datetime.now().isoformat()
        reading = Reading(datetime=dt, co2=reading["CO2"], tvoc=reading["TVOC"])
        session = Session()
        session.add(reading)
        session.commit()
    except Exception as e:
        logger.error(f"failed to save reading: {reading}")


